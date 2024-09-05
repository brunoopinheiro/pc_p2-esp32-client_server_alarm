from src.server.database.abstract_db import Database
from json import load, dumps
from pathlib import Path


class JSONDatabase(Database):

    def __init__(self) -> None:
        self.__basepath = Path(
            '.\\src\\server\\database',
            'database.json',
        )
        bankdict = self.__load_json()
        self.__bankdict: dict = bankdict

    def __load_json(self) -> dict:
        with open(self.__basepath, 'r') as file:
            jsonobject = load(file)
            return jsonobject

    def __update_bank(self):
        jsonstr = dumps(self.__bankdict, indent=4, sort_keys=True)
        with open(self.__basepath, 'w') as outfile:
            outfile.write(jsonstr)

    def __get_last_id(self, table_name: str) -> int:
        table: dict = self.__bankdict.get(table_name)
        last_id = 0
        for key in table.keys():
            last_id = key
        return last_id

    def select(self, table, id):
        table_contents = self.__bankdict.get(table)
        row = table_contents.get(id)
        return row

    def select_by(self, table, column_name, value):
        table_contents: dict = self.__bankdict.get(table)
        for key, row in table_contents.items():
            if row.key() == column_name and row.value() == value:
                return {key: row}

    def insert(self, table, **key_values):
        next_id = self.__get_last_id(table) + 1
        self.__bankdict[table][next_id] = key_values
        self.__update_bank()

    def update(self, table, id, **key_values):
        self.__bankdict[table][id] = key_values
        self.__update_bank()

    def delete(self, table, id):
        updated_dict = self.__bankdict.copy()
        del updated_dict[table][id]
        self.__bankdict = updated_dict
        self.__update_bank()
