from setuptools import setup


def readme():
      with open('README.md') as f:
            return f.read()

setup(name='oppanalyzer',
      version='0.1',
      description='OMNeT++ results analysis tool',
      long_description=readme(),
      author='Stefan Schuhbäck',
      author_email='stefan.schuhbaeck@hm.edu',
      license='MIT',
      packages=['oppanalyzer'],
      zip_safe=False)
