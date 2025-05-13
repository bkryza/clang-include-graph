# Building releases

* Update CHANGELOG.md
* Tag the release commit, e.g. ```git tag 0.1.0```

## Ubuntu

```bash
cd packaging
make DIST=plucky deb
make DIST=focal deb
make DIST=jammy deb
make DIST=noble deb
make DIST=oracular deb

# Repeat for each distro
cd _BUILD/ubuntu/focal
dput ppa:bkryza/clang-include-graph *.changes

```

## Anaconda

```bash
docker run --rm -v $PWD:$PWD continuumio/miniconda3 bash
conda install conda-build make anaconda-client
cd packaging
git config --global --add safe.directory $PWD/..
make CONDA_TOKEN=<CONDA_TOKEN> conda
```