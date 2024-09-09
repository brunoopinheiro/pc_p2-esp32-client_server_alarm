from abc import ABC, abstractmethod


class Database(ABC):

    @abstractmethod
    def select(self, table, id):
        raise NotImplementedError

    @abstractmethod
    def select_by(self, table, column_name, value):
        raise NotImplementedError

    @abstractmethod
    def insert(self, table, **key_values) -> int:
        raise NotImplementedError

    @abstractmethod
    def update(self, table, **key_values):
        raise NotImplementedError

    @abstractmethod
    def delete(self, table, id):
        raise NotImplementedError
