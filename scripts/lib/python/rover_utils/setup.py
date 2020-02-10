from setuptools import setup, find_packages


def readme():
    with open("README.md") as f:
        return f.read()


def requirements():
    with open("requirements.txt") as f:
        ret = [l.strip() for l in f.readlines()]
    return ret


setup(
    name="rover_utils",
    version=0.2,
    description="Collection of tools used for development in roVer",
    long_description=readme(),
    author="Stefan Schuhbäck",
    author_email="stefan.schuhbaeck@hm.edu",
    license="MIT",
    packages=find_packages(),
    install_requires=requirements(),
    zip_safe=False,
)
