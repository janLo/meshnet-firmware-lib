from setuptools import setup, find_packages

setup(
    name='automation_mesh',
    version='0.1',
    packages=find_packages(".", exclude=["tests"]),
    url='https://github.com/janLo/automation_mesh',
    license='BSD',
    author='Jan Losinski',
    author_email='losinskij@gmail.com',
    description='Create a mesh network for IoT',
    install_requires=[
        "pySerial",
        "pyserial-asyncio",
        "siphashc3",
        "colorlog",
        "voluptuous",
        "pyyaml",
    ]
)
