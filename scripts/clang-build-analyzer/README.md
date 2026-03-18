This script used the ClangBuildAnalyzer tool to review clang build profiling information.

- Build and install the following in your path: https://github.com/aras-p/ClangBuildAnalyzer
- Ensure you are building with a clang compiler
- Ensure you run your build with `export VC_CXX_FLAGS="-ftime-trace"`
- Build the code using cmake
- Run the script, example for Tiberian Dawn Debug build: `./analyze-build.sh "tiberian-dawn-debug" "tiberiandawn" "NCO-TD"`
- Review the txt file shown in the output
