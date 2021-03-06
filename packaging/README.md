# Building releases

* Update CHANGELOG.md
* Tag the release commit, e.g. ```git tag 0.1.0```

## Ubuntu

```bash
cd packaging
make DIST=bionic deb
make DIST=focal deb
make DIST=jammy deb

cd _BUILD/ubuntu/focal
dput ppa:bkryza/clang-include-graph *.changes

cd _BUILD/ubuntu/jammy
dput ppa:bkryza/clang-include-graph *.changes

```

## Anaconda

```bash
docker run --rm -v $PWD:$PWD continuum/miniconda3 bash
conda install conda-build make
cd packaging
make CONDA_TOKEN=<CONDA_TOKEN> conda
```