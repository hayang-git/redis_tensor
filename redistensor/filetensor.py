import numpy as np
import os

class FileTensor:
    def __init__(self, path:str) -> None:
        super().__init__()

        if not os.path.exists(path):
            raise Exception("Path does not exist")
        if path.endswith('/'):
            self.path = path
        else:
            self.path = path + '/'

    def shape_to_str(self, shape:tuple):
        return str(shape).replace(' ', '')
    
    def str_to_shape(self, shape:str):
        return tuple(map(int, filter(None, shape[1:-1].split(','))))

    def set(self, key:str, tensor:np.ndarray):
        key_shape = key + "_shape"
        key_dtype = key + "_dtype"
        key_tensor = key + "_tensor"

        try:
            with open(self.path + key_tensor, "wb") as f:
                f.write(tensor.tobytes())
            with open(self.path + key_shape, "w") as f:
                f.write(self.shape_to_str(tensor.shape))
            with open(self.path + key_dtype, "w") as f:
                f.write(str(tensor.dtype))
        except Exception as e:
            print(e)
            raise Exception("Failed to set tensor")

    def get(self, key:str):
        key_shape = key + "_shape"
        key_dtype = key + "_dtype"
        key_tensor = key + "_tensor"
        try:
            with open(self.path + key_dtype, "r") as f:
                dtype = f.read()
            with open(self.path + key_tensor, "rb") as f:
                tensor = np.frombuffer(f.read(), dtype=dtype)
            with open(self.path + key_shape, "r") as f:
                shape = self.str_to_shape(f.read())
            return tensor.reshape(shape)
        except Exception as e:
            print(e)
            raise Exception("Failed to get tensor")

    def names(self, pattern="*_tensor"):
        names = os.listdir(self.path)
        result = []
        for name in names:
            if name.endswith("_tensor"):
                result.append(name[:-7])
        return result
