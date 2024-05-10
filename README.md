# save numpy tensor using redis

## prepare
install redis-server

## using
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


