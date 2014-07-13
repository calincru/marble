# Marble - KDE

It is a copy of [Marble](http://marble.kde.org/)(which is part of KDE) and contains all my work on my GSoC Project.

# How to install

1. Clone the repository
```bash
git clone https://github.com/crucerucalin/marble ~/marble/

```

2. Create the installation directory
```bash
mkdir ~/marble-build
```

2. Run cmake to generate Makefiles
```bash
cd ~/marble-build
cmake ~/marble -DCMAKE_INSTALL_PREFIX=~/marble_bin -DLIB_SUFFIX=64
```

3. Run `make install` to get it installed and that's it. Just run ~/marble_bin/bin/marble if you are on `KDE` or ~/marble_bin/bin/marble-qt if you only have installed `Qt libraries`.
