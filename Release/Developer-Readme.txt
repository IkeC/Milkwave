If you run the Visual Studio solution (Milkwave.sln) in DEBUG configuration, this /Release folder is used as base folder (working directory) for both the Visualizer and the Remote. To set up this folder with default config files and required resources, you can run (or inspect) Developer-Setup.cmd. There's no automatic event for this, so any changes you make eg. to config files are not overwritten while developing.

If you run the Visual Studio solution in RELEASE configuration, the base folder is that of the resulting executable. The file /Build/release.cmd is run as a post-build event, copying the executables, other required files and default config files to the /Release folder.

There are also working tasks and configurations to build and release the solution using VS Code.