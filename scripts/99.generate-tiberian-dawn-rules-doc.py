#!/usr/bin/env python3
"""
Python script to deserialize dictionaries from JSON files 
and render the Jinja template for Tiberian Dawn rules.
"""

import json
import os

from jinja2 import Template

def load_json_files(directory):
    """Load all JSON files from the specified directory."""
    sections = []
    
    # Load each JSON file
    for filename in sorted(os.listdir(directory)):
        if filename.endswith('.json'):
            filepath = os.path.join(directory, filename)
            with open(filepath, 'r') as f:
                data = json.load(f)
                sections.append(data)
    
    return sections

def render_template(template_path, sections):
    """Render the Jinja template with the loaded sections."""
    with open(template_path, 'r') as f:
        template_content = f.read()
    
    # Create Jinja template
    template = Template(template_content)
    
    # Render with sections
    rendered = template.render(sections=sections)
    return rendered

def main():
    """Main function to deserialize JSON files and render template."""
    # Get the directory where this script is located
    script_dir = os.path.dirname(os.path.abspath(__file__))
    
    # Define paths relative to script location
    json_directory = os.path.join(script_dir, '../tiberiandawn/rules')
    template_path = os.path.join(script_dir, '../wiki/2b.Tiberian-Dawn-Rules.md.j2')
    output_path = os.path.join(script_dir, '../wiki/2b.Tiberian-Dawn-Rules.md')

    # Load sections from JSON files
    sections = load_json_files(json_directory)
    
    # Render the template with sections
    rendered_content = render_template(template_path, sections)
    
    # Write to output file instead of printing to stdout
    with open(output_path, 'w') as f:
        f.write(rendered_content)

if __name__ == '__main__':
    main()
