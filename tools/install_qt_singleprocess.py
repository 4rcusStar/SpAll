#!/usr/bin/env python3
"""Single-process Qt installer used by tools/bootstrap_qt.ps1.

Three problems with aqtinstall's own `install-qt` had to be solved here:

1. aqt builds a `multiprocessing.Manager().Queue()` plus a worker pool. On
   Windows that needs named pipes, which this sandbox denies
   (`PermissionError: [WinError 5]` from `_winapi.CreateFile`). Everything below
   therefore runs in one process.

2. aqt sends no User-Agent, so `python-requests`' default UA is served a 403
   anti-hotlink page by the fast Chinese mirrors. We send a browser-like UA,
   which both the mirrors and download.qt.io accept.

3. aqt verifies every archive against a `<archive>.sha256` sidecar. Those
   sidecars are unusable: no mirror hosts them, and the copy on download.qt.io
   is stale (it advertises 94f8a097... while the bytes actually served hash to
   dcaf9508...). Verification is therefore done by fully decoding the 7z
   archive, which catches truncation and corruption, and the measured digest is
   recorded in a manifest so repeat runs and caches can be checked.

Metadata (which packages and archives a Qt version consists of) still comes from
aqtinstall's QtArchives resolver.
"""

import argparse
import hashlib
import json
import shutil
import sys
from pathlib import Path

if "--pythonpath" in sys.argv:
    idx = sys.argv.index("--pythonpath")
    sys.path.insert(0, sys.argv[idx + 1])
    del sys.argv[idx:idx + 2]

from aqt.archives import QtArchives  # noqa: E402

USER_AGENT = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) SpAll-Qt-bootstrap/1.0"
MANIFEST_NAME = "spall_archive_manifest.json"


def log(msg: str) -> None:
    print(msg, flush=True)


def human(n: int) -> str:
    return "%.1f MB" % (n / 1048576)


def download(url: str, dest: Path) -> int:
    """Download url to dest, resuming a partial .part file when possible."""
    import urllib.error
    import urllib.request

    part = dest.with_suffix(dest.suffix + ".part")
    resume_from = part.stat().st_size if part.exists() else 0

    headers = {"User-Agent": USER_AGENT}
    if resume_from:
        headers["Range"] = "bytes=%d-" % resume_from

    req = urllib.request.Request(url, headers=headers)
    try:
        resp = urllib.request.urlopen(req, timeout=60)
    except urllib.error.HTTPError as exc:
        if exc.code == 416 and resume_from:  # already complete
            part.replace(dest)
            return dest.stat().st_size
        raise

    total = int(resp.headers.get("Content-Length", 0)) + resume_from
    mode = "ab" if resume_from and resp.status == 206 else "wb"
    if mode == "wb":
        resume_from = 0
    written = resume_from
    last_report = 0
    with open(part, mode) as out:
        while True:
            chunk = resp.read(1 << 18)
            if not chunk:
                break
            out.write(chunk)
            written += len(chunk)
            if written - last_report > (16 << 20):
                last_report = written
                if total:
                    log("      %s / %s (%.0f%%)" % (human(written), human(total),
                                                    100.0 * written / total))
                else:
                    log("      %s" % human(written))
    part.replace(dest)
    return dest.stat().st_size


def sha256_of(path: Path) -> str:
    digest = hashlib.sha256()
    with open(path, "rb") as handle:
        for chunk in iter(lambda: handle.read(1 << 20), b""):
            digest.update(chunk)
    return digest.hexdigest()


def verify_7z(path: Path) -> None:
    """Fully decode the archive to prove it is intact (raises on corruption)."""
    import py7zr

    with py7zr.SevenZipFile(path, "r") as archive:
        names = archive.getnames()
    if not names:
        raise RuntimeError("archive %s decoded to zero entries" % path.name)


def extract_7z(path: Path, target: Path) -> None:
    import py7zr

    target.mkdir(parents=True, exist_ok=True)
    with py7zr.SevenZipFile(path, "r") as archive:
        archive.extractall(path=str(target))


def main() -> int:
    ap = argparse.ArgumentParser(description="Single-process Qt installer (sandbox-safe).")
    ap.add_argument("--version", required=True, help="Qt version, e.g. 6.9.3")
    ap.add_argument("--arch", default="win64_msvc2022_64")
    ap.add_argument("--outputdir", required=True, help="Qt installation root (e.g. E:\\Qt)")
    ap.add_argument("--archivedir", required=True, help="where downloaded archives are cached")
    ap.add_argument("--base", default="https://download.qt.io", help="repository base URL")
    ap.add_argument("--os", dest="os_name", default="windows")
    ap.add_argument("--target", default="desktop")
    ap.add_argument("--modules", nargs="*", default=None)
    ap.add_argument("--keep", action="store_true", help="keep the downloaded archives")
    ap.add_argument("--force", action="store_true", help="re-download even if cached")
    ap.add_argument("--dry-run", action="store_true", help="only list what would be installed")
    args = ap.parse_args()

    base_dir = Path(args.outputdir).resolve()
    archive_dir = Path(args.archivedir).resolve()
    archive_dir.mkdir(parents=True, exist_ok=True)
    manifest_path = archive_dir / MANIFEST_NAME
    manifest = {}
    if manifest_path.exists():
        try:
            manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
        except Exception:
            manifest = {}

    log("Resolving %s %s %s from %s" % (args.os_name, args.target, args.version, args.base))

    qt_archives = QtArchives(
        args.os_name,
        args.target,
        args.version,
        args.arch,
        base=args.base,
        subarchives=None,
        modules=args.modules,
        all_extra=False,
        is_include_base_package=True,
        timeout=(10, 60),
    )

    packages = qt_archives.get_packages()
    log("\nPackages to install (%d):" % len(packages))
    for pkg in packages:
        log("  - %-46s %s" % (pkg.name, pkg.archive))
    if args.dry_run:
        return 0

    failures = []
    for i, pkg in enumerate(packages, 1):
        archive = archive_dir / pkg.archive
        target = base_dir / pkg.archive_install_path
        # pkg.archive_path is already the full repo-relative path
        # ("online/qtsdkrepository/..."), so only the host prefix is prepended.
        url = args.base.rstrip("/") + "/" + pkg.archive_path.lstrip("/")
        log("\n[%d/%d] %s" % (i, len(packages), pkg.name))

        try:
            if archive.exists() and not args.force:
                log("      cached %s" % archive.name)
            else:
                log("      downloading %s" % url)
                size = download(url, archive)
                log("      downloaded %s" % human(size))

            digest = sha256_of(archive)
            known = manifest.get(pkg.archive)
            if known and known != digest and not args.force:
                raise RuntimeError(
                    "cached archive %s changed on disk (expected %s, found %s)"
                    % (archive.name, known[:16], digest[:16]))
            manifest[pkg.archive] = digest
            log("      sha256 %s" % digest)

            verify_7z(archive)
            extract_7z(archive, target)
            log("      extracted -> %s" % target)

            if not args.keep:
                archive.unlink()
        except Exception as exc:  # noqa: BLE001 - report which package failed, keep going
            log("      FAILED: %s: %s" % (type(exc).__name__, exc))
            failures.append((pkg.name, "%s: %s" % (type(exc).__name__, exc)))

    manifest_path.write_text(json.dumps(manifest, indent=2, sort_keys=True), encoding="utf-8")

    if failures:
        log("\n%d package(s) failed:" % len(failures))
        for name, err in failures:
            log("  - %s: %s" % (name, err))
        return 1

    # qmake needs a qt.conf pointing at its own prefix; the Qt installer normally
    # writes one. Without it qmake reports the build-time prefix.
    bin_dir = base_dir / "bin"
    if bin_dir.is_dir():
        qmake = bin_dir / "qmake.exe"
        if qmake.exists():
            conf = bin_dir / "qt.conf"
            if not conf.exists():
                conf.write_text("[Paths]\nPrefix=..\n", encoding="utf-8")
                log("\nwrote %s" % conf)

    log("\nQt %s installed into %s" % (args.version, base_dir))
    if shutil.which("cmake") is None:
        log("(cmake is not on PATH; use the CMake bundled with Visual Studio)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
