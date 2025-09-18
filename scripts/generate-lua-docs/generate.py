#!/usr/bin/env python3
"""
Python script to transform Lua type documentation JSON file 
and render the Jinja template for Lua API reference.
"""

import json
import os

from jinja2 import Template


def char_range(c1: str, c2: str):
  for c in range(ord(c1), ord(c2)+1):
    yield chr(c)

def render_template(template_path: str, packages: dict):
  print(f"Rendering jinja template file: {template_path}")

  if not os.path.exists(template_path):
    raise FileNotFoundError(f"Markdown template file not found @ {template_path}")

  with open(template_path, 'r') as f:
      template_content = f.read()

  a_to_z_upper = list(char_range("A", "Z"))
  
  sections = {
    "Global Objects": "globals",
    "Classes": "classes",
    "Types": "types"
  }

  template = Template(template_content)
  rendered = template.render(packages=packages, sections=sections, a_to_z_upper=a_to_z_upper)

  return rendered

def should_include_define(define: dict, prefix: str, exclude: str = "zzzzzz") -> bool:
  result = False

  if not "file" in define:
    return result

  if not define["file"].startswith(exclude):
    result = define["file"].startswith(prefix)

  return result

def filter_type_doc(type_doc: dict, internal_types: list[str], prefix: str, exclude: str = "zzzzzz",  force_types: list[str] = []) -> dict:
  types = list(
    filter(
      lambda t: 
        (t["type"] == "type" or t["name"] in force_types) and "defines" in t and any(
          should_include_define(d, prefix, exclude) for d in t["defines"]
        ),
      type_doc
    )
  )

  return {
    "classes": list(filter(
      lambda t: ("/lib/" in t["defines"][0]["file"]) and t["defines"][0]["file"].endswith(f"{t["name"]}.lua") and (t["name"] not in internal_types),
      types
    )),
    "globals": list(filter(
      lambda t: (not "/lib/" in t["defines"][0]["file"]) and t["defines"][0]["file"].endswith(f"{t["name"]}.lua") and (t["name"] not in internal_types),
      types
    )),
    "types": list(filter(
      lambda t: (not "/lib/" in t["defines"][0]["file"]) and (not t["defines"][0]["file"].endswith(f"{t["name"]}.lua")) and (t["name"] not in internal_types),
      types
    ))
  }

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
  
  internal_types = [
    "ApiModule",
    "CncApiMock"
  ]

  # TODO: Move types to bottom of each package group
  packages = {
    "nco": filter_type_doc(type_doc, internal_types, common_prefix, exclude=common_lib_prefix, force_types=["Rules"]),
    "nco.TiberianDawn": filter_type_doc(type_doc, internal_types, td_prefix, td_lib_prefix),
    "nco.lib": filter_type_doc(type_doc, internal_types, common_lib_prefix),
    "nco.TiberianDawn.lib": filter_type_doc(type_doc, internal_types, td_lib_prefix)
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
