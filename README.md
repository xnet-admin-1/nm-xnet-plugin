# nm-xnet-plugin

NetworkManager VPN plugin for XNet.

## Overview

Python-based plugin that wraps `xnet-cli` to integrate XNet into Linux desktop network management via NetworkManager.

## Install

1. Copy plugin files to the appropriate NetworkManager directories.
2. Restart NetworkManager:

```sh
sudo systemctl restart NetworkManager
```

## Dependencies

- NetworkManager
- Python 3
- `xnet-cli` (from [xnet-core](../xnet-core))

## License

Apache 2.0
