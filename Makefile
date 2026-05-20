CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS = -lpthread -lrt
TARGET = interseccion
SRC = interseccion.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

fase1: $(TARGET)
	@echo "Ejecutando FASE 1 (sin sincronizacion)..."
	./$(TARGET)

fase2: $(TARGET)
	@echo "Ejecutando FASE 2 (con sincronizacion)..."
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all clean fase1 fase2
