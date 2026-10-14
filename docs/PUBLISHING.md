# Publishing SpAll

Two independent paths. Either one produces the same public result: `main` plus the
`v1.0.0` tag on GitHub, and a source-only Release.

The repository is `https://github.com/4rcusStar/SpAll`. At the time of writing it exists
but is empty, and the fine-grained token in use is **read-only** — every write returns
`403 Resource not accessible by personal access token`.

## Fix the token first (needed by both paths)

GitHub → Settings → Developer settings → **Fine-grained tokens** → this token:

- **Repository access** → include `SpAll`
- **Permissions** → **Contents: Read and write**

That single permission is the only thing missing. `Metadata: Read` is granted
automatically; a classic token with the `repo` scope also works.

## Path 1 — push from your own machine (no token sharing needed)

`build/SpAll.bundle` is a self-contained git bundle containing the full history plus the
tag. It was produced with `git bundle create --all` and verified with
`git bundle verify` ("The bundle records a complete history").

```sh
git clone SpAll.bundle SpAll
cd SpAll
git remote set-url origin https://github.com/4rcusStar/SpAll.git
git push origin main --tags
```

Or, to push straight from the existing working copy instead of the bundle:

```sh
cd <path-to>/SpAll
git remote set-url origin https://github.com/4rcusStar/SpAll.git
git push origin main --tags
```

## Path 2 — let the publish script do it

```sh
python tools/publish_github.py --repo 4rcusStar/SpAll --token <PAT>
```

The script pushes the whole tree through the **Git Data API** rather than `git push`, then
creates the tag, the Release and the asset upload. Add `--dry-run` to see the full
sequence without writing anything.

It uses the API for a specific reason: on the machine this was developed on, only git's
TLS stack could reach github.com. The REST API was unreachable from .NET and from curl
because both use Windows schannel (`schannel: AcquireCredentialsHandle failed:
SEC_E_NO_CREDENTIALS`, on every host), while git is configured with
`http.sslbackend=openssl`. Python uses OpenSSL too, so the API is reachable from Python —
and creating a Release is impossible through the git protocol alone.

## The Release itself

If you use Path 1, opening the Release is a manual step in the browser:

1. GitHub → Releases → **Draft a new release**
2. Choose the existing tag **`v1.0.0`**
3. Title `SpAll 1.0.0`, and paste the body from `docs/release-notes-1.0.0.md`
4. Optionally attach `build/SpAll-1.0.0-source.zip` — GitHub also generates source
   archives automatically, so this is not required

Path 2 does all of the above, including the asset upload.

## What gets published

80 files, ~230 KB. `third_party/`, `.qt/` and `build/` are gitignored, so the dependency
sources and the Qt installation are not included — a clone re-creates them with
`tools/fetch_deps.ps1` and `tools/bootstrap_qt.ps1`.
