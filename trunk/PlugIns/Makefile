SUBDIRS = BSPSceneManager FileSystem ParticleFX

all:
	for sd in $(SUBDIRS); do \
		echo "Building in $$sd"; \
		(cd $$sd && make all) || exit 1; \
	done

clean:
	for sd in $(SUBDIRS); do \
		echo "Cleaning in $$sd"; \
		(cd $$sd && make clean) || exit 1; \
	done
