all:
	pio run --target build

clean:
	pio run --target clean

upload:
	pio run --target upload

update:
	pio run --target compiledb
