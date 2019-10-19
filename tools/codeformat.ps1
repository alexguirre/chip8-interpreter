
function Invoke-ClangFormat() {
    $files = Get-ChildItem "src" -Exclude "build" |
        Get-ChildItem -Recurse -Include *.cpp, *.h |
        Select-Object -ExpandProperty FullName

    Write-Output $files

    & "./tools/clang-format.exe" -i $files
}

Invoke-ClangFormat