#!/usr/bin/env python3
"""Offline wheelhouse builder for SpAll's Qt acquisition step.

Responsibilities:
  * Resolve a package and its transitive dependency closure from PyPI metadata.
  * Select the best-matching wheel for the running interpreter/platform.
  * Download every wheel into a local wheelhouse directory.

This exists because `pip install` cannot reach any package index on this host
(pip's resolver stalls), while plain urllib HTTP downloads work fine. The
wheelhouse it produces is then installed with:
    pip install --no-index --find-links <wheelhouse> <packages>

Only the Python standard library is used.
"""

import argparse
import json
import os
import re
import sys
import time
import urllib.request
from urllib.error import HTTPError, URLError

PYPI_JSON = "https://pypi.org/pypi/{name}/json"
USER_AGENT = "SpAll-wheelhouse/1.0 (offline bootstrap)"

# Markers that are irrelevant for a normal desktop Windows/Linux build.
_SKIP_MARKERS = ("extra ==", "extra ==", "os_name == \"posix\"", "platform_system == \"Darwin\"")


def _log(msg: str) -> None:
    print(msg, flush=True)


def fetch_json(url: str, retries: int = 3):
    """GET a URL as JSON, retrying transient network failures."""
    last = None
    for attempt in range(retries):
        try:
            req = urllib.request.Request(url, headers={"User-Agent": USER_AGENT})
            with urllib.request.urlopen(req, timeout=45) as resp:
                return json.load(resp)
        except (HTTPError, URLError, TimeoutError) as exc:
            last = exc
            if isinstance(exc, HTTPError) and exc.code == 404:
                raise
            time.sleep(1.5 * (attempt + 1))
    raise RuntimeError("cannot fetch %s: %s" % (url, last))


def norm(name: str) -> str:
    """PEP 503 normalized project name."""
    return re.sub(r"[-_.]+", "-", name).lower()


def parse_version(text: str):
    """Parse a PEP 440-ish version into a comparable tuple.

    Pre-release/dev segments sort before the matching release, which is what we
    want when preferring stable versions.
    """
    text = text.strip()
    m = re.match(r"^[vV]?(\d+(?:\.\d+)*)(.*)$", text)
    if not m:
        return ((), 1, text)
    nums = tuple(int(p) for p in m.group(1).split("."))
    rest = m.group(2).lower()
    # rank: dev < a < b < rc < final < post
    rank = 4
    for label, value in (("dev", 0), ("a", 1), ("b", 2), ("rc", 3), ("post", 5)):
        if rest.startswith(label):
            rank = value
            break
    return (nums, rank, rest)


def marker_applies(marker: str) -> bool:
    """Best-effort evaluation of an environment marker for this interpreter."""
    if not marker:
        return True
    m = marker.lower()
    if "extra ==" in m:
        return False  # optional extras are not needed to run aqtinstall
    if "sys_platform" in m:
        want_win = "\"win32\"" in m or "'win32'" in m
        want_linux = "linux" in m
        is_win = sys.platform == "win32"
        if want_win and not is_win:
            return False
        if want_linux and is_win:
            return False
    if "python_version" in m:
        # Handle simple >= / < / == comparisons against the running version.
        cur = tuple(sys.version_info[:2])
        mm = re.search(r"python_version\s*(>=|<=|==|>|<)\s*['\"](\d+)\.(\d+)['\"]", m)
        if mm:
            op, major, minor = mm.group(1), int(mm.group(2)), int(mm.group(3))
            other = (major, minor)
            if op == ">=" and not cur >= other:
                return False
            if op == "<=" and not cur <= other:
                return False
            if op == ">" and not cur > other:
                return False
            if op == "<" and not cur < other:
                return False
            if op == "==" and not cur == other:
                return False
        elif "python_version" in m and "\"3\"" in m and sys.version_info[0] != 3:
            return False
    return True


def requirement_specs(requires_dist):
    """Yield (normalized_name, [specifier, ...]) for *install* dependencies.

    Extras (``; extra == "dev"``) are deliberately excluded: aqtinstall's test
    and documentation dependencies are not needed to run it, and pulling them in
    drags in packages with no wheels for this interpreter.
    """
    out = []
    for raw in requires_dist or []:
        # Split off the marker: name[extras] (specs) ; marker
        if ";" in raw:
            req_part, marker = raw.split(";", 1)
            marker = marker.strip()
            if "extra" in marker.lower():
                continue  # optional extra, not required at runtime
            if not marker_applies(marker):
                continue
        else:
            req_part = raw
        req_part = req_part.split("[")[0].strip()
        m = re.match(r"^([A-Za-z0-9._-]+)\s*(.*)$", req_part)
        if not m:
            continue
        name = norm(m.group(1))
        specs = re.findall(r"(==|>=|<=|!=|~=|>|<)\s*([0-9][0-9A-Za-z.\-+]*)", m.group(2))
        out.append((name, specs))
    return out


def satisfies(version: str, specs) -> bool:
    """Check a version string against a list of (op, value) specifiers."""
    have = parse_version(version)
    for op, want in specs:
        need = parse_version(want)
        if op == "==" and not have[:2] == need[:2]:
            return False
        if op == "!=" and have[:2] == need[:2]:
            return False
        if op == ">=" and have[:2] < need[:2]:
            return False
        if op == "<=" and have[:2] > need[:2]:
            return False
        if op == ">" and have[:2] <= need[:2]:
            return False
        if op == "<" and have[:2] >= need[:2]:
            return False
        if op == "~=" and have[:2] < need[:2]:
            return False
    return True


def _platform_compatible(plattag: str) -> bool:
    """True when a wheel's platform tag is usable on this host."""
    tags = plattag.split(".")
    if "any" in tags:
        return True
    if sys.platform == "win32":
        # win32 wheels are 32-bit but still a valid last resort on x64 hosts;
        # win_amd64 and win_arm64 are the native choices.
        return any(t.startswith("win") for t in tags)
    return any(("manylinux" in t) or ("linux" in t) or t.startswith("macosx") for t in tags)


def _interpreter_compatible(pytag: str, abitag: str) -> bool:
    """PEP 425 style check of a wheel's python/abi tags against this interpreter."""
    cur = (sys.version_info.major, sys.version_info.minor)
    for py in pytag.split("."):
        if py in ("py3", "py2.py3", "py2"):
            if py != "py2" and sys.version_info.major == 3:
                return True
            continue
        m = re.fullmatch(r"cp(\d)(\d+)", py)
        if not m:
            continue
        wheel_ver = (int(m.group(1)), int(m.group(2)))
        if py in ("cp27", "cp36"):
            continue
        if wheel_ver == cur:
            return True
        # abi3 wheels work on this and every later CPython of the same major.
        if "abi3" in abitag.split(".") and wheel_ver <= cur and wheel_ver[0] == cur[0]:
            return True
    return False


def wheel_rank(filename: str):
    """Return a preference tuple for a wheel, or None when it is unusable."""
    parts = filename[:-4].split("-")
    if len(parts) < 5:
        return None
    pytag, abitag, plattag = parts[-3], parts[-2], parts[-1]
    if not _platform_compatible(plattag):
        return None
    if not _interpreter_compatible(pytag, abitag):
        return None
    # Prefer: exact cp-tag over abi3 over pure python; native arch over win32.
    exact_cp = 1 if ("cp%d%d" % (sys.version_info.major, sys.version_info.minor)) in pytag.split(".") else 0
    abi3 = 1 if "abi3" in abitag.split(".") else 0
    native = 1 if ("win_amd64" in plattag.split(".") or "any" in plattag.split(".")) else 0
    pure = 1 if "any" in plattag.split(".") else 0
    return (exact_cp, abi3, native, pure)


def pick_release(info):
    """Choose the newest compatible stable release from a PyPI JSON payload."""
    version = info["info"]["version"]
    files = info.get("urls") or []
    # Prefer the advertised latest version; fall back to the newest release list.
    if version and any(candidates_for(files, version)):
        return version, files
    releases = info.get("releases") or {}
    names = sorted(releases.keys(), key=parse_version, reverse=True)
    for name in names:
        entries = releases[name]
        if not entries:
            continue
        if candidates_for(entries, name):
            return name, entries
    return version, files


def candidates_for(files, version):
    out = []
    for f in files:
        if f.get("packagetype") != "bdist_wheel" and not f["filename"].endswith(".whl"):
            continue
        if f.get("yanked"):
            continue
        rank = wheel_rank(f["filename"])
        if rank is not None:
            out.append((rank, f))
    out.sort(key=lambda item: item[0], reverse=True)
    return out


def download(url: str, dest: str, expected_size: int = 0, retries: int = 3) -> None:
    """Download a file to dest, skipping it when already present and sized."""
    if os.path.exists(dest) and expected_size and os.path.getsize(dest) == expected_size:
        _log("      cached  %s" % os.path.basename(dest))
        return
    tmp = dest + ".part"
    last = None
    for attempt in range(retries):
        try:
            req = urllib.request.Request(url, headers={"User-Agent": USER_AGENT})
            with urllib.request.urlopen(req, timeout=60) as resp, open(tmp, "wb") as out:
                while True:
                    chunk = resp.read(1 << 18)
                    if not chunk:
                        break
                    out.write(chunk)
            os.replace(tmp, dest)
            return
        except Exception as exc:  # noqa: BLE001 - report and retry any transport error
            last = exc
            time.sleep(1.5 * (attempt + 1))
    raise RuntimeError("cannot download %s: %s" % (url, last))


def resolve(roots, wheelhouse: str) -> list:
    """Resolve the dependency closure of `roots` and download its wheels."""
    os.makedirs(wheelhouse, exist_ok=True)
    pending = [(norm(r), []) for r in roots]
    chosen = {}          # name -> (version, wheel_file)
    constraints = {}     # name -> accumulated specifiers
    failures = []

    while pending:
        name, extra_specs = pending.pop(0)
        constraints.setdefault(name, [])
        constraints[name].extend(extra_specs)

        if name in chosen:
            # Re-validate against newly learned constraints.
            ver, _ = chosen[name]
            if not satisfies(ver, constraints[name]):
                _log("  !! %s %s conflicts with %s; keeping first pick" % (name, ver, constraints[name]))
            continue

        try:
            info = fetch_json(PYPI_JSON.format(name=name))
        except HTTPError as exc:
            failures.append("%s: PyPI metadata unavailable (%s)" % (name, exc.code))
            continue

        version, files = pick_release(info)
        if not satisfies(version, constraints[name]):
            releases = info.get("releases") or {}
            for cand in sorted(releases.keys(), key=parse_version, reverse=True):
                if satisfies(cand, constraints[name]) and candidates_for(releases[cand], cand):
                    version, files = cand, releases[cand]
                    break

        cands = candidates_for(files, version)
        if not cands:
            failures.append("%s %s: no compatible wheel" % (name, version))
            continue

        _, wheel = cands[0]
        chosen[name] = (version, wheel)
        _log("  -> %-24s %-12s %s" % (name, version, wheel["filename"]))

        dest = os.path.join(wheelhouse, wheel["filename"])
        download(wheel["url"], dest, wheel.get("size") or 0)

        for dep_name, dep_specs in requirement_specs(info["info"].get("requires_dist")):
            if dep_name not in chosen:
                pending.append((dep_name, dep_specs))
            else:
                constraints.setdefault(dep_name, []).extend(dep_specs)

    return failures


def main() -> int:
    ap = argparse.ArgumentParser(description="Build an offline wheelhouse for SpAll's Qt bootstrap.")
    ap.add_argument("--wheelhouse", required=True, help="directory to place downloaded wheels")
    ap.add_argument("packages", nargs="+", help="root requirements, e.g. aqtinstall")
    args = ap.parse_args()

    wheelhouse = os.path.abspath(args.wheelhouse)
    _log("wheelhouse: %s" % wheelhouse)
    _log("python: %s %s" % (sys.version.split()[0], sys.platform))

    failures = resolve(args.packages, wheelhouse)

    wheels = [f for f in os.listdir(wheelhouse) if f.endswith(".whl")]
    total = sum(os.path.getsize(os.path.join(wheelhouse, f)) for f in wheels)
    _log("\ncollected %d wheels, %.1f MB" % (len(wheels), total / 1048576))
    if failures:
        _log("FAILURES:")
        for f in failures:
            _log("  - %s" % f)
        return 1
    _log("OK")
    return 0


if __name__ == "__main__":
    sys.exit(main())
