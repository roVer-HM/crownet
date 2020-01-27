from setuptools import setup


def readme():
    with open("README.md") as f:
        return f.read()


def requirements():
    with open("requirements.txt") as f:
        ret = [l.strip() for l in f.readlines()]
    return ret


setup(
    name="oppanalyzer",
    version=1.0,
    description="OMNeT++ results analysis tool",
    long_description=readme(),
    author="Stefan Schuhbäck",
    author_email="stefan.schuhbaeck@hm.edu",
    license="MIT",
    packages=["oppanalyzer", "tutorial"],
    install_requires=requirements(),
    zip_safe=False,
)
