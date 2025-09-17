#!/usr/bin/env python3
"""
Python script to transform Lua type documentation JSON file 
and render the Jinja template for Lua API reference.
"""

import json
import os

from jinja2 import Template

def render_template(template_path: str, packages: dict):
    print(f"Rendering jinja template file: {template_path}")

    if not os.path.exists(template_path):
      raise FileNotFoundError(f"Markdown template file not found @ {template_path}")

    with open(template_path, 'r') as f:
        template_content = f.read()

    template = Template(template_content)
    rendered = template.render(packages=packages)

    return rendered

def should_include_define(define: dict, prefix: str, exclude: str = "zzzzzz") -> bool:
  result = False

  if not "file" in define:
    return result

  if not define["file"].startswith(exclude):
    result = define["file"].startswith(prefix)

  return result

def filter_type_doc(type_doc: dict, prefix: str, exclude: str = "zzzzzz") -> list[dict]:
  return list(
    filter(
      lambda t: 
        t["type"] == "type" and "defines" in t and any(
          should_include_define(d, prefix, exclude) for d in t["defines"]
        ),
      type_doc
    )
  )

def load_type_doc(root_dir: str) -> dict:
  data_path = os.path.join(
    root_dir,
    "scripts/generate-lua-docs/doc.json"
  )

  if not os.path.exists(data_path):
    raise FileNotFoundError(f"Type doc json file not found @ {data_path}")

  with open(data_path, 'r') as file:
    return json.load(file)

def main():
  common_prefix="common/lua/scripts/nco"
  common_lib_prefix=f"{common_prefix}/lib"
  td_prefix="tiberiandawn/lua/scripts/nco/TiberianDawn"
  td_lib_prefix=f"{td_prefix}/lib"

  root_dir = os.path.abspath(
    os.path.join(
        os.path.dirname(os.path.abspath(__file__)),
        "../.."
    )
  )

  type_doc = load_type_doc(root_dir)

  # TODO: Move types to bottom of each package group
  packages = {
    "nco": filter_type_doc(type_doc, common_prefix, common_lib_prefix),
    "nco.TiberianDawn": filter_type_doc(type_doc, td_prefix, td_lib_prefix),
    "nco.lib": filter_type_doc(type_doc, common_lib_prefix),
    "nco.TiberianDawn.lib": filter_type_doc(type_doc, td_lib_prefix)
  }

  # render markdown template
  output_path = os.path.join(root_dir, 'wiki/3c.Lua-API-Reference.md')
  template_path = f"{output_path}.j2"
  
  rendered_content = render_template(template_path, packages)

  print(f"Writing rendered template to file: {output_path}")

  with open(output_path, 'w') as f:
      f.write(rendered_content)

if __name__ == '__main__':
    main()
