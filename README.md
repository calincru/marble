# Marble - KDE

It is a copy of [Marble](http://marble.kde.org/) (which is part of KDE) and
contains all my work on my *GSoC Project*. A more detailed description of what
my work involves can be found on [my blog](http://calincruceru.net/).

## How to install

1. Clone the repository
    ```bash
    git clone https://github.com/crucerucalin/marble ~/marble/
    ```

2. Create the installation directory
    ```bash
    mkdir ~/marble-build
    ```

3. Run **cmake** to generate Makefiles
    ```bash
    cd ~/marble-build
    cmake ~/marble -DCMAKE_INSTALL_PREFIX=~/marble_bin -DLIB_SUFFIX=64
    ```

4. Run `make install` to get it installed and that's it. Launch it using `~/marble_bin/bin/marble` if you are using *KDE* or `~/marble_bin/bin/marble-qt` if you only have installed **Qt libraries**.

5. To have the **Annotate Plugin** working, make sure that `[plugin_annotation]` from `~/.kde/share/config/marblerc` has both *enabled* and *visible* fields set to **true**.
