
ALLDIRS  = nr_server nr_client tools dbiface tools/fontmaker

all: compile

install:
	@for dir in $(ALLDIRS); do \
		$(MAKE) -C $$dir $@ || exit 1; \
	done

compile:
	mkdir -p so 2>/dev/null
	@for dir in ${ALLDIRS}; do \
		cd $$dir; \
		${MAKE} || exit 1; \
		cd - ;\
	done

clean:
	@for dir in ${ALLDIRS}; do \
		cd $$dir; \
		${MAKE} -e clean || exit 1; \
		cd - ;\
	done

