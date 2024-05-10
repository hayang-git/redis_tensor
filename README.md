# 功能说明


## 安装
1. redis-server安装
sudo apt-get install redis-server

2. redistensor模块安装
pip install redistensor

## 使用示例
```python
from redistensor import RedisTensor as rt
import numpy as np

host = "localhost"
port = 6379
password = ""
r = rt(host, port, password)

# save one tensor
x = np.random.randn(2,3).astype(np.float32)
print(x)

r.set('x_1', x)

# get one tensor
y = r.get('x_1')
print(y)

# redis tensor name list
names = r.names()

for name in names:
    print(name)
```


