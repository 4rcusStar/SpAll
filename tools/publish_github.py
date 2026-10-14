#!/usr/bin/env python3
"""Publish SpAll to GitHub: push the tree, tag it, and open a Release.

Why a script instead of `git push` plus the web UI: on the machine this was developed
on, only git's TLS stack could reach github.com at all, and `api.github.com` was
unreachable from .NET and from curl because both use Windows schannel while git uses
OpenSSL. Python uses OpenSSL too, so the REST API is reachable from here — which is what
makes creating a Release and uploading an asset possible at all.

The token needs, at minimum:
  * Contents: Read and write   (to push the tree and create the Release)
  * Metadata: Read             (granted automatically)

Usage:
    python tools/publish_github.py --repo 4rcusStar/SpAll --token <PAT> [--dry-run]

The push is deliberately performed through the Git Data API rather than `git push`, so
it works even where the git transport is unavailable or blocked. Existing refs are moved
with force=false, so a non-fast-forward is reported instead of silently discarding
history.
"""

import argparse
import base64
import json
import mimetypes
import os
import subprocess
import sys
import urllib.error
import urllib.request

API = "https://api.github.com"


class GitHub:
    def __init__(self, token: str, dry_run: bool = False):
        self.token = token
        self.dry_run = dry_run

    def request(self, path, method="GET", data=None, raw=None, content_type=None):
        url = path if path.startswith("https://") else API + path
        body = None
        headers = {
            "Authorization": "Bearer " + self.token,
            "User-Agent": "SpAll-publish",
            "Accept": "application/vnd.github+json",
            "X-GitHub-Api-Version": "2022-11-28",
        }
        if raw is not None:
            body = raw
            headers["Content-Type"] = content_type or "application/octet-stream"
        elif data is not None:
            body = json.dumps(data).encode()
            headers["Content-Type"] = "application/json"

        if self.dry_run and method != "GET":
            print(f"  [dry-run] {method} {path}")
            return None

        req = urllib.request.Request(url, method=method, data=body, headers=headers)
        try:
            with urllib.request.urlopen(req, timeout=120) as resp:
                payload = resp.read().decode()
                return json.loads(payload) if payload.strip() else None
        except urllib.error.HTTPError as exc:
            detail = exc.read().decode()[:400]
            raise SystemExit(f"ERROR {method} {path} -> HTTP {exc.code}\n{detail}")


def git(*args) -> str:
    return subprocess.run(["git", *args], check=True, capture_output=True, text=True).stdout


def tracked_files() -> list[str]:
    return [line for line in git("ls-files").splitlines() if line.strip()]


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--repo", required=True, help="owner/name")
    parser.add_argument("--token", default=os.environ.get("GITHUB_TOKEN", ""))
    parser.add_argument("--branch", default="main")
    parser.add_argument("--tag", default="v1.0.0")
    parser.add_argument("--title", default="SpAll 1.0.0")
    parser.add_argument("--notes", default="build/release-notes.md")
    parser.add_argument("--asset", default="build/SpAll-1.0.0-source.zip")
    parser.add_argument("--dry-run", action="store_true")
    args = parser.parse_args()

    if not args.token:
        raise SystemExit("No token. Pass --token or set GITHUB_TOKEN.")

    owner, _, name = args.repo.partition("/")
    if not name:
        raise SystemExit("--repo must be owner/name")

    gh = GitHub(args.token, args.dry_run)

    # --- preflight: confirm the token can actually write ---------------------
    print(f"repo: {args.repo}")
    me = gh.request("/user")
    print(f"  authenticated as: {me.get('login')}")
    repo = gh.request(f"/repos/{owner}/{name}")
    print(f"  exists: {repo.get('full_name')}  private={repo.get('private')}  "
          f"default_branch={repo.get('default_branch')}")

    # --- build the tree through the Git Data API -----------------------------
    files = tracked_files()
    print(f"uploading {len(files)} files as blobs ...")
    tree = []
    for index, path in enumerate(files, 1):
        with open(path, "rb") as handle:
            payload = handle.read()
        blob = gh.request(f"/repos/{owner}/{name}/git/blobs", "POST",
                          {"content": base64.b64encode(payload).decode(), "encoding": "base64"})
        sha = blob["sha"] if blob else "0" * 40
        tree.append({"path": path.replace(os.sep, "/"), "mode": "100644",
                     "type": "blob", "sha": sha})
        if index % 20 == 0 or index == len(files):
            print(f"  {index}/{len(files)}")

    print("creating tree ...")
    new_tree = gh.request(f"/repos/{owner}/{name}/git/trees", "POST", {"tree": tree})
    tree_sha = new_tree["sha"] if new_tree else "0" * 40

    message = git("log", "-1", "--pretty=%B").strip() or "SpAll 1.0.0"
    print("creating commit ...")
    commit = gh.request(f"/repos/{owner}/{name}/git/commits", "POST",
                        {"message": message, "tree": tree_sha, "parents": []})
    commit_sha = commit["sha"] if commit else "0" * 40
    print(f"  commit {commit_sha[:10]}")

    print(f"setting refs/heads/{args.branch} ...")
    existing = None
    try:
        existing = gh.request(f"/repos/{owner}/{name}/git/ref/heads/{args.branch}")
    except SystemExit:
        existing = None
    if existing:
        gh.request(f"/repos/{owner}/{name}/git/refs/heads/{args.branch}", "PATCH",
                   {"sha": commit_sha, "force": False})
    else:
        gh.request(f"/repos/{owner}/{name}/git/refs", "POST",
                   {"ref": f"refs/heads/{args.branch}", "sha": commit_sha})

    # --- tag + release -------------------------------------------------------
    print(f"creating tag {args.tag} ...")
    tag_obj = gh.request(f"/repos/{owner}/{name}/git/tags", "POST",
                         {"tag": args.tag, "message": f"SpAll {args.tag.lstrip('v')}",
                          "object": commit_sha, "type": "commit"})
    if tag_obj:
        gh.request(f"/repos/{owner}/{name}/git/refs", "POST",
                   {"ref": f"refs/tags/{args.tag}", "sha": tag_obj["sha"]})

    with open(args.notes, encoding="utf-8") as handle:
        notes = handle.read()

    print(f"creating release {args.tag} ...")
    release = gh.request(f"/repos/{owner}/{name}/releases", "POST",
                         {"tag_name": args.tag, "name": args.title, "body": notes,
                          "draft": False, "prerelease": False})

    # --- asset ---------------------------------------------------------------
    if release and os.path.exists(args.asset):
        upload_url = release["upload_url"].split("{")[0]
        with open(args.asset, "rb") as handle:
            payload = handle.read()
        ctype = mimetypes.guess_type(args.asset)[0] or "application/zip"
        print(f"uploading {os.path.basename(args.asset)} ({len(payload)} bytes) ...")
        gh.request(upload_url + "?name=" + os.path.basename(args.asset), "POST",
                   raw=payload, content_type=ctype)

    if release:
        print("\nrelease:", release["html_url"])
    else:
        print("\n[dry-run] no changes were made")
    return 0


if __name__ == "__main__":
    sys.exit(main())
