import setuptools

with open('README.md', 'r') as f:
    long_description = f.read()

setuptools.setup(
    name="redistensor",
    version="0.0.2",
    author="hayang",
    author_email="469358331@qq.com",
    description="using redis to store tensor data",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="",
    packages=setuptools.find_packages(),
    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
    ],
    python_requires='>=3.6',
    install_requires=[
        'numpy',
        'redis',
    ],
)
