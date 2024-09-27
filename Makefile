# Node.js addon build commands

# Ensure node-gyp is installed
check-node-gyp:
	@command -v node-gyp >/dev/null 2>&1 || { echo >&2 "node-gyp is required but not installed. Install it using: npm install -g node-gyp"; exit 1; }

# Configure the build
configure: check-node-gyp
	npx node-gyp configure

# Build the addon
build: configure
	npx node-gyp build
	cp build/Release/yyjson.node lib/

# Clean the build
clean:
	rm -rf build
	npx node-gyp clean

# Rebuild the addon (clean and build)
rebuild: clean build

# Install dependencies
install-deps:
	npm install

# Run tests (assuming you have a test script in package.json)
test: build
	npm test

# Default target
all: build

.PHONY: check-node-gyp configure build clean rebuild install-deps test all