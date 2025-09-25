#!/usr/bin/env python3
"""
Python script to generate a rockspec file based on current
Lua modules in the repo.
"""

import fnmatch
import os
import sys

from jinja2 import Template

def render_template(template_path: str, spec_version: str, packages: dict[str, str]):
    """Render the Jinja template with the available packages."""
    print(f"Rendering jinja template file: {template_path}")

    if not os.path.exists(template_path):
      raise FileNotFoundError(f"Markdown template file not found @ {template_path}")

    with open(template_path, 'r') as f:
        template_content = f.read()
    
    # Create Jinja template
    template = Template(template_content)

    # Render with sections
    rendered = template.render(version=spec_version, packages=packages)
    return rendered

def find_files_with_extension(directory: str, extension: str):
    matched_files = []
    for root, dirs, files in os.walk(directory):
        for filename in fnmatch.filter(files, f'*.{extension}'):
            matched_files.append(os.path.join(root, filename))
    return matched_files

def scan_lua_tree(root_path: str, tree_root_path: str, packages: dict[str, str]):
    lua_files = find_files_with_extension(tree_root_path, "lua")
    
    # Extract the package name from the file path, convert from x/y/z.lua to nco.x.y.z
    for file in lua_files:
        package_name = os.path.relpath(file, tree_root_path)

        # Lua package.path has pattern for init.lua if parent directory matches require string: `?/init.lua`
        # So, we just remove init.lua when parsing package Lua path
        if package_name.endswith("init.lua"):
            package_name = package_name.replace(f"{os.sep}init.lua", "").replace(f"init.lua", "")

            if package_name != "":
                package_name = f".{package_name.replace(os.sep, '.')}"
        else:
            package_name = f".{package_name.replace(".lua", "").replace(os.sep, '.')}"

        packages[f"nco{package_name}"] = os.path.relpath(file, root_path)

def main():
    """Main function to search for Lua files and render template."""
    spec_version = len(sys.argv) > 1 and sys.argv[1] or "scm-0"

    # Get the directory where this script is located
    root_dir = os.path.abspath(
        os.path.join(
            os.path.dirname(os.path.abspath(__file__)),
            "../.."
        )
    )

    # Define paths relative to script location
    common_lua_directory = os.path.join(root_dir, 'common/lua/scripts/nco')
    td_lua_directory = os.path.join(root_dir, 'tiberiandawn/lua/scripts/nco')
    output_path = os.path.join(root_dir, f'cnc-nco-{spec_version}.rockspec')
    template_path = os.path.join(root_dir, 'cnc-nco.rockspec.j2')

    packages = {}

    scan_lua_tree(root_dir, common_lua_directory, packages)
    scan_lua_tree(root_dir, td_lua_directory, packages)

    # Render the template with packages
    rendered_content = render_template(template_path, spec_version, packages)

    # Write to output file instead of printing to stdout
    print(f"Writing rendered template to file: {output_path}")

    with open(output_path, 'w') as f:
        f.write(rendered_content)

if __name__ == '__main__':
    main()
