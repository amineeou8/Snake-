# Compilateur et options
CC = gcc
CFLAGS = -std=c99 -Wall

# Ton fichier source (l'IA)
SRC = OudjaneAmine.c

# Nom de l'exécutable final
EXEC = snake

# --- CHOIX DE L'ARCHITECTURE (Décommenter la ligne qui correspond à votre PC) ---

# Pour Linux (PC Classique Intel/AMD)
OBJ = snake-intel-linux.o

# Pour Linux (Architecture ARM / Raspberry Pi)
# OBJ = snake-arm-linux.o

# Pour Mac OS (Puce Intel)
# OBJ = snake-intel-macos.o

# Pour Mac OS (Puce Apple M1/M2/M3)
# OBJ = snake-arm-macos.o

# -----------------------------------------------------------------------------

all: $(EXEC)

$(EXEC): $(SRC)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJ) $(SRC)

clean:
	rm -f $(EXEC) player.o
