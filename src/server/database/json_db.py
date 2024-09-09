from __future__ import annotations
from server.database.abstract_db import Database
from json import load, dumps
from pathlib import Path


class JSONDatabase(Database):

    __instance = None

    def __new__(cls):
        if not cls.__instance:
            cls.__instance = super(JSONDatabase, cls).__new__(cls)
        return cls.__instance

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
            last_id = int(key)
        return last_id

    def select(self, table, id):
        table_contents = self.__bankdict.get(table)
        row = table_contents.get(id)
        result_dict = row.copy()
        result_dict['id'] = id
        return result_dict

    def select_by(self, table, column_name, value):
        table_contents: dict = self.__bankdict.get(table)
        result = []
        for id, row in table_contents.items():
            for column, cvalue in row.items():
                if column == column_name and cvalue == value:
                    result_dict = row.copy()
                    result_dict['id'] = id
                    result.append(result_dict)
        return result

    def insert(self, table, **key_values):
        next_id = self.__get_last_id(table) + 1
        self.__bankdict[table][str(next_id)] = key_values
        self.__update_bank()
        return next_id

    def insert_known_id(self, table, id, **key_values):
        try:
            self.__bankdict[table][str(id)] = key_values
            self.__update_bank()
        except Exception as err:
            print("Error: ", err)

    def update(self, table, id, **key_values):
        self.__bankdict[table][id] = key_values
        self.__update_bank()

    def delete(self, table, id):
        updated_dict = self.__bankdict.copy()
        del updated_dict[table][id]
        self.__bankdict = updated_dict
        self.__update_bank()
