import json
import jsonschema
import click

def verify_worker(schema_path: str, json_path: str) -> None:
    'validates a json file with a schema'
    with open(schema_path) as fobj:
        schema = json.load(fobj)
    jsonschema.Draft7Validator.check_schema(schema)
    with open(json_path) as fobj:
        instance = json.load(fobj)
    jsonschema.Draft7Validator(schema).validate(instance)

def verify(schema_path: str, json_path: str) -> bool:
    'returns True if the json file is consistent with the schema'
    try:
        verify_worker(schema_path, json_path)
        return True
    except Exception as e:
        print(e)
        return False

@click.command()
@click.option(
    '--schema',
    type=str,
    help='path to JSON schema file',
    required=True,
    prompt=True
    )
@click.option(
    '--json',
    type=str,
    help='path to JSON file to be verified',
    required=True,
    prompt=True
    )
def main(schema, json) -> None:
    '''
Verifies a JSON file against a schema.

> python jverify.py --schema <schema> --json <json>

where <json> is the path to the JSON schema file
and <schema> is the path to the JSON file to be verified.
    '''
    fmt_good = "\"{1}\" obeys the schema \"{0}\""
    fmt_bad = "{1} and {0} are not consistent"
    if verify(schema_path=schema, json_path=json):
        fmt = fmt_good
    else:
        fmt = fmt_bad
    print(fmt.format(schema, json))

if __name__ == '__main__':
    main()
