# PageGoat

PageGoat is a C17-based website discovery and intelligence extraction engine. It currently supports single-website downloading and basic metadata, contact, and branding extraction.

## Requirements

- Windows
- MSYS2 UCRT64 environment
- GCC
- CMake
- Ninja
- libcurl

Ensure the MSYS2 UCRT64 compiler and dependencies are installed and available to CMake.

## Build and Run

### 1. Open the repository

Open PowerShell in the PageGoat root directory, where `CMakeLists.txt` is located.

```powershell
cd D:\Work\NewYou\pagegoat
```

### 2. Configure the build

Run this for the first build or whenever the build directory has been deleted:

```powershell
cmake -S . -B build -G Ninja
```

### 3. Compile the application

```powershell
cmake --build build
```

For detailed compiler and linker output:

```powershell
cmake --build build --verbose
```

### 4. Run PageGoat

From the repository root:

```powershell
.\build\pagegoat.exe
```

Enter a website when prompted, for example:

```text
apple.com
```

### 5. Rebuild after source changes

From the repository root:

```powershell
cmake --build build
```

Then run:

```powershell
.\build\pagegoat.exe
```

### 6. Clean rebuild

To delete generated build files and recreate them from scratch, run these commands from the repository root:

```powershell
Remove-Item .\build -Recurse -Force
cmake -S . -B build -G Ninja
cmake --build build --verbose
.\build\pagegoat.exe
```

**Important:** The delete command removes the existing `build` directory. Run it only from the repository root and only when you intend to perform a clean rebuild.

### 7. Build and run from inside the build directory

If your terminal is already inside `build`:

```powershell
cmake --build .
.\pagegoat.exe
```

Do not run `cmake --build build` from inside the build directory, because that looks for a nested `build\build` directory.

## Current Status

- [x] C17/CMake project configuration
- [x] CLI input for a website URL
- [x] URL normalization (initial implementation)
- [x] HTTP download using libcurl
- [x] Basic HTML metadata extraction
- [x] Initial contact and branding extraction
- [ ] Reliable URL validation and HTTP error handling
- [ ] Accurate contact extraction
- [ ] Technology and SEO detection
- [ ] Company/search-query discovery
- [ ] URL queue and single-worker processing
- [ ] Unified structured result output
- [ ] JSON and CSV export
- [ ] Frontend integration

## Development Principle

Stabilize and test the C engine before building the frontend. Keep discovery, downloading, parsing, extraction, and exporting as separate components.

### START DEV

PS D:\Work\NewYou\pagegoat> cmake --build build

PS D:\Work\NewYou\pagegoat> .\build\pagegoat.exe       

PS D:\Work\NewYou\pagegoat> .\build\pagegoat-service.exe   

This should open - 
[INFO] HTTP initialized.
PageGoat service listening at http://127.0.0.1:8787
Endpoint: POST http://127.0.0.1:8787/extract

Then in next terminal (new)

PS D:\Work\NewYou\pagegoat> cd .\apps\web
PS D:\Work\NewYou\pagegoat\apps\web> npm run dev

> web@0.1.0 dev
> next dev

▲ Next.js 16.4.2 (Turbopack)
- Local:         http://localhost:3000
- Network:       http://192.168.100.220:3000
- Environments: .env.local
✓ Ready in 6.7s
✓ Running next.config.ts took 424ms