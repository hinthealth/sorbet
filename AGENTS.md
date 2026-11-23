# hinthealth/sorbet

Hint's fork of [sorbet/sorbet](https://github.com/sorbet/sorbet). It carries typechecker fixes
upstream hasn't taken yet and publishes them as gems that
[hint-api](https://github.com/hinthealth/hint-api) installs. Upstream's own docs (`README.md`,
`docs/`, `website/`) still apply to the code; this file covers only what the fork adds.

## Branches

- `master` mirrors sorbet/sorbet. Only GitHub's "Sync fork" touches it. Never commit to it.
- `hint` (the default branch) is `master` plus our fixes. A ruleset makes every change a pull
  request, squash-merged, so each fix is one commit. Force-pushes are allowed: rebasing `hint` is
  how we take upstream updates.
- `fix/<slug>`: one branch per fix, off `master`, so the same commit can also be sent upstream.
- `release/<upstream version>`: our fixes replayed onto the upstream release hint-api uses, e.g.
  `release/0.6.13616`. Release tags go here.

## Adding a fix

1. Branch `fix/<slug>` off `master`.
2. Change the typechecker. Every fix needs a test in `test/testdata/` (a `.rb` file with
   `# error:` annotations; see upstream's `docs/internals.md`). If it changes how untyped values
   are counted, also extend `test/cli/track-untyped/`.
3. Build: `./bazel build //main:sorbet --config=release-mac` (Linux: `--config=release-linux`).
   Take about 12 minutes from clean; later builds are incremental.
4. Check it against hint-api: in a hint-api checkout,
   `SRB_SORBET_EXE=<path to bazel-bin/main/sorbet> bundle exec srb tc`. Compare errors and
   `--track-untyped` counts with the pinned gem. New errors in hint-api must be fixed there, in a
   hint-api PR that passes with both the pinned gem and the new build, before the fix ships.
5. Open a PR into `hint` and squash-merge it. Optionally send the same commit upstream as a PR
   to sorbet/sorbet from the `fix/` branch.

Measured on hint-api, for reference: the three fixes on `hint` today removed about 4,400 untyped
usages and left typecheck time unchanged.

## Releasing

1. Replay the fixes onto the release branch for the upstream version hint-api uses:
   `git switch release/<upstream>` and `git cherry-pick -x <squash commits from hint>`.
2. Tag `hint-v<upstream>.<n>`, where `<n>` counts our releases on that upstream version
   (`hint-v0.6.13616.2`), and push the branch and the tag.
3. `.github/workflows/hint-release.yml` checks that the tag matches the branch's upstream base,
   builds `sorbet-static` for x86_64-linux and universal-darwin plus `sorbet` and
   `sorbet-static-and-runtime` at that version, and force-pushes a static gem index to
   `gh-pages`, keeping the last 6 versions. `sorbet-runtime` is not built; ours requires
   upstream's.
4. In hint-api, bump the three pins in the `source 'https://raw.githubusercontent.com/hinthealth/sorbet/gh-pages'`
   block of the `Gemfile`, run `bundle lock`, and open the PR.

Our versions add a fourth number on purpose. Bundler reuses an installed gem by name, version and
platform, so publishing under upstream's exact version would silently keep a stock
`sorbet-static` on machines that already had it.

## Moving to a new upstream version

1. "Sync fork" on `master`.
2. Rebase `hint` onto `master` and force-push. A fix upstream has merged drops out by itself.
3. Cut `release/<new upstream>` from the upstream commit for that version (its commit count on
   upstream's master is the last number of the version), cherry-pick the fixes, tag
   `hint-v<new upstream>.1`.
4. In hint-api, bump the three pins and `sorbet-runtime` together. Dependabot ignores all four
   Sorbet gems, so nothing else will do it.

## Pitfalls we hit

- **Stale incremental builds.** After switching branches, an incremental Bazel build once
  produced a binary whose prebuilt stdlib data was out of date: every block parameter typed as
  `Integer(0)`, then a segfault on hint-api. Smoke-test a new build with
  `["a"].each { |x| T.reveal_type(x) }` (expect `String`); if it's wrong, `./bazel clean` and
  rebuild.
- **`case/in` bindings are untyped** on the released builds. Code that needs the type uses
  `case/when`, which narrows.
- **GitHub Packages can't host these gems**: it allows one gem per version, and we publish two
  platforms per version. That is why the index lives on `gh-pages`.
- The inherited `ci`, `codeql` and `delete-pr-branch` workflows only act when the repository is
  sorbet/sorbet (or Shopify's fork), so they do nothing here. Leave them; deleting them makes
  every upstream sync conflict.

## Don'ts

- Don't commit to `master`, and don't merge into `hint` without a PR.
- Don't push anything Hint-specific to sorbet/sorbet PRs: code and tests only, no hint-api names
  or numbers.
- Don't edit `gh-pages` by hand except to remove a bad release; the release workflow owns it.
