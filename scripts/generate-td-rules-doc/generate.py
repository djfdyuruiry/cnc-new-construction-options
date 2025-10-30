#!/usr/bin/env python3
"""
Python script to deserialize dictionaries from JSON files 
and render the Jinja template for Tiberian Dawn rules.
"""

import json
import os

from jinja2 import Template

def render_template(template_path, sections, concrete_types):
    """Render the Jinja template with the loaded sections."""
    print(f"Rendering jinja template file: {template_path}")

    if not os.path.exists(template_path):
      raise FileNotFoundError(f"Markdown template file not found @ {template_path}")

    with open(template_path, 'r') as f:
        template_content = f.read()
    
    # Create Jinja template
    template = Template(template_content)

    # Render with sections
    rendered = template.render(sections=sections, concrete_types=concrete_types)
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

    # substitute boolean for yes/no
    for section in sections:
        for rule in section['rules']:
            if type(rule['default']) == bool:
                rule['default'] = rule['default'] and "yes" or "no"

    # Fill in defaults for type properties (examples, valid values etc.)
    for game_type in types:
        for prop in game_type['properties']:
            if not prop.get('comment', False):
                prop['comment'] = "-"

            if not prop.get('type', False):
                continue

            # determine example value using valid values list (defaults to NONE)
            if prop.get('requires_converter', False) or prop.get('requires_csv_converter', False):
                example_values = prop['valid_values'].split(',')
                example_value = example_values[0]

                if len(example_values) > 1:
                    example_value = example_values[1]

                if example_value == prop['valid_values']:
                    example_value = 'NONE'

                prop['example_value'] = prop.get('example_value', example_value).strip()

                continue

            # determine example value and valid values using type
            match prop['type']:
                case 'Bool':
                    prop['example_value'] = prop.get('example_value', "yes")
                    prop['valid_values'] = prop.get('valid_values', "yes, no")
                case 'Int':
                    prop['example_value'] = prop.get('example_value', 10)
                    prop['valid_values'] = prop.get('valid_values', "Range: -2147483648 to 2147483647")
                case 'UInt':
                    prop['example_value'] = prop.get('example_value', 10)
                    prop['valid_values'] = prop.get('valid_values', "Range: 0 to 4294967295")
                case 'UShort':
                    prop['example_value'] = prop.get('example_value', 10)
                    prop['valid_values'] = prop.get('valid_values', "Range: 0 to 65535")
                case 'Char':
                    prop['example_value'] = prop.get('example_value', 10)
                    prop['valid_values'] = prop.get('valid_values', "Range: 0 to 255")
                case 'UChar':
                    prop['example_value'] = prop.get('example_value', 10)
                    prop['valid_values'] = prop.get('valid_values', "Range: -128 to 127")
                case 'String':
                    prop['example_value'] = prop.get('example_value', "'a string'")
                    prop['valid_values'] = prop.get('valid_values', "anything")

        if not game_type.get('base_class', False):
            continue

        # link types together backed on class and base_class fields
        game_type['has_base_type'] = True
        game_type['base_type'] = next((t for t in types if t['class'] == game_type['base_class']), None)

    # We don't want to render base types (these are usually abstract)
    concrete_types = list(filter(lambda t: not t.get('is_base_class', False), types))

    # Render the template with sections
    rendered_content = render_template(template_path, sections, concrete_types)

    # Write to output file instead of printing to stdout
    print(f"Writing rendered template to file: {output_path}")

    with open(output_path, 'w') as f:
        f.write(rendered_content)

if __name__ == '__main__':
    main()
