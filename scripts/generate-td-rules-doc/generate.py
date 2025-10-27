#!/usr/bin/env python3
"""
Python script to deserialize dictionaries from JSON files 
and render the Jinja template for Tiberian Dawn rules.
"""

import json
import os

from jinja2 import Template

def render_template(template_path, sections, base_types, concrete_types):
    """Render the Jinja template with the loaded sections."""
    print(f"Rendering jinja template file: {template_path}")

    if not os.path.exists(template_path):
      raise FileNotFoundError(f"Markdown template file not found @ {template_path}")

    with open(template_path, 'r') as f:
        template_content = f.read()
    
    # Create Jinja template
    template = Template(template_content)

    # Render with sections
    rendered = template.render(sections=sections, base_types=base_types, concrete_types=concrete_types)
    return rendered

def load_json_files(directory):
    """Load all JSON files from the specified directory."""
    data_from_files = []
    
    print(f"Loading rule data from files: {directory}/*.json")

    # Load each JSON file
    for filename in sorted(os.listdir(directory)):
        if filename.endswith('.json'):
            filepath = os.path.join(directory, filename)
            with open(filepath, 'r') as f:
                data = json.load(f)
                data_from_files.append(data)
    
    return data_from_files

def main():
    """Main function to deserialize JSON files and render template."""
    # Get the directory where this script is located
    root_dir = os.path.abspath(
        os.path.join(
            os.path.dirname(os.path.abspath(__file__)),
            "../.."
        )
    )

    # Define paths relative to script location
    rules_json_directory = os.path.join(root_dir, 'tiberiandawn/rules')
    types_json_directory = os.path.join(root_dir, 'tiberiandawn/types')
    output_path = os.path.join(root_dir, 'wiki/2b.Tiberian-Dawn-Rules.md')
    template_path = f"{output_path}.j2"

    # Load sections from JSON files
    sections = load_json_files(rules_json_directory)
    types = load_json_files(types_json_directory)

    for type in types:
        if not type.get('base_class', False):
            continue

        type['has_base_type'] = True
        type['base_type'] = next((t for t in types if t['class'] == type['base_class']), None)

        ## TODO: generate example_value based on type (and if requires converter then second in list of valid_values or NONE)
        ## TODO: generate valid_values based on type, for non converter types (number range, true/false or 'any')

    concrete_types = list(filter(lambda t: not t.get('is_base_class', False), types))
    base_types = list(filter(lambda t: t.get('is_base_class', False), types))

    # Render the template with sections
    rendered_content = render_template(template_path, sections, base_types, concrete_types)

    # Write to output file instead of printing to stdout
    print(f"Writing rendered template to file: {output_path}")

    with open(output_path, 'w') as f:
        f.write(rendered_content)

if __name__ == '__main__':
    main()
