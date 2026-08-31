CXX ?= g++
CXXFLAGS ?= -O3 -std=c++17 -Wall -Wextra -Wpedantic -Werror -fopenmp
BIN := bin/symmetric_network_matrix

.PHONY: all verify verify-reference structure-audit coordinate-audit coordinate-audit-production recipient-order-audit reviewer-gap smoke fast full sanitizer scale-fast clean clean-generated

all: verify

$(BIN): code/symmetric_network_matrix.cpp code/network_kernels.hpp
	@mkdir -p bin results
	$(CXX) $(CXXFLAGS) code/symmetric_network_matrix.cpp -o $(BIN)

smoke: $(BIN)
	./$(BIN) smoke

structure-audit: $(BIN)
	python3 tests/test_network_structure.py

coordinate-audit: $(BIN)
	python3 verification/coordinate_realization_audit.py --seeds 2 --threads 2 --families 20 --steps 400 --n 80 --offsets 0 1 2

coordinate-audit-production: $(BIN)
	python3 verification/coordinate_realization_audit.py --seeds 12 --threads $$(nproc) --families 1000 --steps 12000 --n 4000 --offsets 0 1 2 3

recipient-order-audit:
	python3 verification/directed_recipient_order_audit.py

reviewer-gap: coordinate-audit
	python3 verification/reviewer_gap_report.py

verify: $(BIN) structure-audit
	./$(BIN) verify
	python3 tests/test_suite.py

sanitizer: code/symmetric_network_matrix.cpp
	@mkdir -p bin
	$(CXX) -O1 -g -std=c++17 -Wall -Wextra -Wpedantic -Werror -fopenmp -fsanitize=address,undefined -fno-omit-frame-pointer code/symmetric_network_matrix.cpp -o bin/symmetric_network_matrix_san
	ASAN_OPTIONS=detect_leaks=1 UBSAN_OPTIONS=print_stacktrace=1 ./bin/symmetric_network_matrix_san smoke

fast: $(BIN)
	@mkdir -p results
	THREADS="$${THREADS:-$$(nproc)}"; SEEDS="$${SEEDS:-12}"; FAMILIES="$${FAMILIES:-1000}"; STEPS="$${STEPS:-12000}"; N="$${N:-4000}"; MIX_STRENGTH="$${MIX_STRENGTH:-1.0}"; SHOCK_INTERVAL="$${SHOCK_INTERVAL:-500}"; SYMPTOM_WINDOW="$${SYMPTOM_WINDOW:-0.06}"; \
	OMP_NUM_THREADS=$$THREADS ./$(BIN) matrix $$SEEDS $$THREADS $$FAMILIES $$STEPS $$N $$MIX_STRENGTH $$SHOCK_INTERVAL $$SYMPTOM_WINDOW | tee results/fast.log

full: $(BIN)
	@mkdir -p results
	THREADS="$${THREADS:-$$(nproc)}"; SEEDS="$${SEEDS:-56}"; FAMILIES="$${FAMILIES:-1000}"; STEPS="$${STEPS:-12000}"; N="$${N:-4000}"; MIX_STRENGTH="$${MIX_STRENGTH:-1.0}"; SHOCK_INTERVAL="$${SHOCK_INTERVAL:-500}"; SYMPTOM_WINDOW="$${SYMPTOM_WINDOW:-0.06}"; \
	OMP_NUM_THREADS=$$THREADS ./$(BIN) matrix $$SEEDS $$THREADS $$FAMILIES $$STEPS $$N $$MIX_STRENGTH $$SHOCK_INTERVAL $$SYMPTOM_WINDOW | tee results/full.log

scale-fast: $(BIN)
	@set -eu; THREADS="$${THREADS:-$$(nproc)}"; SEEDS="$${SEEDS:-12}"; STEPS="$${STEPS:-12000}"; N="$${N:-4000}"; MIX_STRENGTH="$${MIX_STRENGTH:-1.0}"; \
	for F in 1000 500 250 100; do \
		test $$((N % F)) -eq 0; mkdir -p results/families_$$F; \
		echo "--- families=$$F family_size=$$((N/F)) ---"; \
		OMP_NUM_THREADS=$$THREADS ./$(BIN) matrix $$SEEDS $$THREADS $$F $$STEPS $$N $$MIX_STRENGTH | tee results/families_$$F/run.log; \
		cp results/matrix_per_seed.csv results/families_$$F/; cp results/matrix_summary.csv results/families_$$F/; cp results/differences.csv results/families_$$F/; \
	done

verify-reference: verify
	python3 reproduce/verify_preflight.py

clean-generated:
	rm -rf bin results

clean: clean-generated
