#!/bin/bash

# Set installation path
INSTALL_DIR="$HOME/sgoinfre/Postman"
POSTMAN_URL="https://dl.pstmn.io/download/latest/linux64"
POSTMAN_TARBALL="postman.tar.gz"

# Check if Postman is already installed
if [ -f "$INSTALL_DIR/Postman" ]; then
    echo "Postman is already installed at $INSTALL_DIR. Skipping installation."
else
    # Create directory if it doesn't exist
    mkdir -p "$INSTALL_DIR"

    # Download Postman
    echo "Downloading Postman..."
    wget -q --show-progress "$POSTMAN_URL" -O "$POSTMAN_TARBALL"

    # Extract to sgoinfre/Postman
    tar -xvf "$POSTMAN_TARBALL" -C "$INSTALL_DIR" --strip-components=1

    # Clean up tarball
    rm "$POSTMAN_TARBALL"

    echo "Postman installed at $INSTALL_DIR."
fi

# Create alias for zsh and optionally bash
ALIAS_CMD="alias postman=\"$INSTALL_DIR/Postman\""

# Update .zshrc
if grep -q "alias postman=" ~/.zshrc; then
    echo "Updating existing postman alias in .zshrc..."
    sed -i "s|alias postman=.*|$ALIAS_CMD|" ~/.zshrc
else
    echo "Adding postman alias to .zshrc..."
    echo "$ALIAS_CMD" >> ~/.zshrc
fi

# Confirm installation
echo "Type 'postman' to run. If 'postman' is not recognized, restart your shell or run 'source ~/.zshrc'."
