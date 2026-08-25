# Zero Package Manager (zpkg) User Manual

`zpkg` is the package manager for LinuxOSZero, managing software installation, updates, and removals.

## Command Reference

### 1. List Installed Packages
```bash
zpkg list
```

### 2. Search Available Packages
```bash
zpkg search <query>
```
*Example:* `zpkg search vbox`

### 3. Package Information
```bash
zpkg info <package_name>
```
*Example:* `zpkg info zero-wm`

### 4. Install a Package
```bash
zpkg install <package_name>
```
*Example:* `zpkg install gcc-toolchain`

### 5. Remove a Package
```bash
zpkg remove <package_name>
```
*Example:* `zpkg remove zero-editor`

### 6. Update Repositories
```bash
zpkg update
```
