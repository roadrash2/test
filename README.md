# My First Mod (Geode)

1. Install Geode CLI + SDK: https://docs.geode-sdk.org
2. Set the GEODE_SDK environment variable.
3. In this folder run: `geode build`
4. Copy the generated `.geode` file into your Geode `mods` folder (or run `geode install`).

Edit `mod.json` (id, developer, versions) to match your setup. Compare with a fresh `geode new` project if the versions are rejected.

## No local setup: build the .geode on GitHub
1. Create a free GitHub repo and upload this whole folder (including the hidden .github folder).
2. Open the repo's Actions tab, run "Build Mod" (or just push).
3. When it finishes, download "Build Output" from the run page. It contains the .geode file.
