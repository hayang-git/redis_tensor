import numpy as np
import redis

class RedisTensor:
    def __init__(self, host:str='localhost', port:int=6379, password: str= None) -> None:
        super().__init__()
        self.host = host
        self.port = port
        self.password = password
        self.db = None

        self.str_type = {
            'float32': np.float32,
            'float64': np.float64,
            'int32': np.int32,
            'int64': np.int64,
            'uint8': np.uint8,
            'uint16': np.uint16,
            'uint32': np.uint32,
            'uint64': np.uint64,
            'int8': np.int8,
            'int16': np.int16,
            'bool': np.bool_,
            'complex64': np.complex64,
            'complex128': np.complex128
        }
        try:
            self.db = redis.Redis(host=self.host, port=self.port, password=self.password)
        except Exception as e:
            print(e)
            raise Exception("Failed to connect to Redis")

    def shape_to_str(self, shape:tuple):
        return str(shape).replace(' ', '')
    
    def str_to_shape(self, shape:str):
        return tuple(map(int, shape[1:-1].split(',')))
    
    def set(self, key:str, tensor:np.ndarray):
        key_shape = key + "_shape"
        key_dtype = key + "_dtype"
        key_tensor = key + "_tensor"

        try:
            self.db.set(key_tensor, tensor.tobytes())
            self.db.set(key_shape, self.shape_to_str(tensor.shape))
            self.db.set(key_dtype, str(tensor.dtype))
        except Exception as e:
            print(e)
            raise Exception("Failed to set tensor")

    def get(self, key:str):
        key_shape = key + "_shape"
        key_dtype = key + "_dtype"
        key_tensor = key + "_tensor"
        try:
            tensor = self.db.get(key_tensor)
            if tensor is None:
                raise Exception("Tensor not found")
            dtype = self.str_type[self.db.get(key_dtype).decode('utf-8')]
            shape = self.str_to_shape(self.db.get(key_shape).decode('utf-8'))
            return np.frombuffer(tensor, dtype=dtype).reshape(shape)
        except Exception as e:
            print(e)
            raise Exception("Failed to get tensor")
    
    def names(self, pattern="*_tensor"):
        names = self.db.keys(pattern)
        result = []
        for name in names:
            tmpname = name.decode('utf-8')
            result.append(tmpname[:-7])
        return result
    
    def get_tensor_shape(self, key:str):
        try:
            tensor = self.get(key)
            return tensor.shape
        except Exception as e:
            print(e)
            raise Exception("Failed to get tensor shape")

if __name__ == "__main__":
    host = "localhost"
    port = 6379
    password = ""
    rt = RedisTensor(host, port, password)
    
    #x = np.random.rand(2, 3).astype(np.float32)

    #rt.set_tensor("x0", x)

    y = rt.get_tensor_shape("x0")

    print(y)
        
