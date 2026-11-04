#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define NUM_NODES 5
#define MAX_BACKOFF 10
#define TOTAL_SLOTS 100
#define P_TRANSMIT 0.3

typedef struct {
    int id;
    int backoff_counter;
    int collisions;
    int successful_packets;
    int attempted_packets;
} Node;

void reset_nodes(Node nodes[], int count) {
    for (int i = 0; i < count; i++) {
        nodes[i].id = i + 1;
        nodes[i].backoff_counter = 0;
        nodes[i].collisions = 0;
        nodes[i].successful_packets = 0;
        nodes[i].attempted_packets = 0;
    }
}

// 1. Pure ALOHA Simulation
void simulate_pure_aloha(int total_time) {
    printf("\n=== PURE ALOHA SIMULATION ===\n");
    Node nodes[NUM_NODES];
    reset_nodes(nodes, NUM_NODES);

    int total_success = 0;
    int total_collisions = 0;

    for (int t = 0; t < total_time; t++) {
        int active_nodes = 0;
        int transmitting_node = -1;

        for (int i = 0; i < NUM_NODES; i++) {
            double r = (double)rand() / RAND_MAX;
            if (r < P_TRANSMIT) {
                active_nodes++;
                transmitting_node = i;
                nodes[i].attempted_packets++;
            }
        }

        if (active_nodes == 1) {
            nodes[transmitting_node].successful_packets++;
            total_success++;
            printf("[Time %02d] Success: Node %d transmitted successfully.\n", t, nodes[transmitting_node].id);
        } else if (active_nodes > 1) {
            total_collisions++;
            printf("[Time %02d] Collision! %d nodes tried to transmit simultaneously.\n", t, active_nodes);
        }
    }

    printf("\nPure ALOHA Results:\n");
    printf("Total Slots: %d | Successful Transmissions: %d | Collisions: %d\n", total_time, total_success, total_collisions);
    printf("Throughput: %.2f%%\n", ((double)total_success / total_time) * 100.0);
}

// 2. Slotted ALOHA Simulation
void simulate_slotted_aloha(int total_slots) {
    printf("\n=== SLOTTED ALOHA SIMULATION ===\n");
    Node nodes[NUM_NODES];
    reset_nodes(nodes, NUM_NODES);

    int total_success = 0;
    int total_collisions = 0;

    for (int slot = 0; slot < total_slots; slot++) {
        int transmitting_count = 0;
        int last_node = -1;

        for (int i = 0; i < NUM_NODES; i++) {
            double r = (double)rand() / RAND_MAX;
            if (r < P_TRANSMIT) {
                transmitting_count++;
                last_node = i;
                nodes[i].attempted_packets++;
            }
        }

        if (transmitting_count == 1) {
            nodes[last_node].successful_packets++;
            total_success++;
            printf("[Slot %02d] Success: Node %d transmitted.\n", slot, nodes[last_node].id);
        } else if (transmitting_count > 1) {
            total_collisions++;
            printf("[Slot %02d] Collision: %d nodes transmitted in slot.\n", slot, transmitting_count);
        }
    }

    printf("\nSlotted ALOHA Results:\n");
    printf("Total Slots: %d | Successful Transmissions: %d | Collisions: %d\n", total_slots, total_success, total_collisions);
    printf("Throughput: %.2f%%\n", ((double)total_success / total_slots) * 100.0);
}

// 3. CSMA/CD Simulation
void simulate_csma_cd(int total_slots) {
    printf("\n=== CSMA/CD SIMULATION ===\n");
    Node nodes[NUM_NODES];
    reset_nodes(nodes, NUM_NODES);

    int total_success = 0;
    int total_collisions = 0;

    for (int slot = 0; slot < total_slots; slot++) {
        int ready_nodes[NUM_NODES];
        int ready_count = 0;

        for (int i = 0; i < NUM_NODES; i++) {
            if (nodes[i].backoff_counter > 0) {
                nodes[i].backoff_counter--;
            } else {
                double r = (double)rand() / RAND_MAX;
                if (r < P_TRANSMIT) {
                    ready_nodes[ready_count++] = i;
                    nodes[i].attempted_packets++;
                }
            }
        }

        if (ready_count == 1) {
            int node_idx = ready_nodes[0];
            nodes[node_idx].successful_packets++;
            nodes[node_idx].collisions = 0;
            total_success++;
            printf("[Slot %02d] Success: Node %d sensed medium FREE & transmitted.\n", slot, nodes[node_idx].id);
        } else if (ready_count > 1) {
            total_collisions++;
            printf("[Slot %02d] Collision Detected! %d nodes tried to transmit.\n", slot, ready_count);

            for (int k = 0; k < ready_count; k++) {
                int idx = ready_nodes[k];
                nodes[idx].collisions++;
                int max_k = (nodes[idx].collisions > MAX_BACKOFF) ? MAX_BACKOFF : nodes[idx].collisions;
                int max_slots = (1 << max_k) - 1;
                nodes[idx].backoff_counter = (max_slots > 0) ? (rand() % max_slots) + 1 : 1;
                printf("          -> Node %d jam signal sent. Backoff set to %d slots.\n", nodes[idx].id, nodes[idx].backoff_counter);
            }
        }
    }

    printf("\nCSMA/CD Results:\n");
    printf("Total Slots: %d | Successful Transmissions: %d | Collisions: %d\n", total_slots, total_success, total_collisions);
    printf("Throughput: %.2f%%\n", ((double)total_success / total_slots) * 100.0);
}

// 4. CSMA/CA Simulation
void simulate_csma_ca(int total_slots) {
    printf("\n=== CSMA/CA SIMULATION ===\n");
    Node nodes[NUM_NODES];
    reset_nodes(nodes, NUM_NODES);

    int total_success = 0;
    int total_collisions = 0;

    for (int i = 0; i < NUM_NODES; i++) {
        nodes[i].backoff_counter = rand() % 8 + 1;
    }

    for (int slot = 0; slot < total_slots; slot++) {
        int transmitting_count = 0;
        int ready_nodes[NUM_NODES];

        for (int i = 0; i < NUM_NODES; i++) {
            if (nodes[i].backoff_counter == 0) {
                ready_nodes[transmitting_count++] = i;
            } else {
                nodes[i].backoff_counter--;
            }
        }

        if (transmitting_count == 1) {
            int idx = ready_nodes[0];
            nodes[idx].successful_packets++;
            total_success++;
            nodes[idx].backoff_counter = rand() % 8 + 1;
            printf("[Slot %02d] CSMA/CA Success: Node %d sent RTS/CTS + Data + ACK successfully.\n", slot, nodes[idx].id);
        } else if (transmitting_count > 1) {
            total_collisions++;
            printf("[Slot %02d] CSMA/CA Collision: RTS Collision between %d nodes!\n", slot, transmitting_count);
            for (int k = 0; k < transmitting_count; k++) {
                int idx = ready_nodes[k];
                nodes[idx].collisions++;
                nodes[idx].backoff_counter = rand() % (8 * (nodes[idx].collisions + 1)) + 1;
                printf("          -> Node %d re-selected backoff window: %d slots.\n", nodes[idx].id, nodes[idx].backoff_counter);
            }
        }
    }

    printf("\nCSMA/CA Results:\n");
    printf("Total Slots: %d | Successful Transmissions: %d | Collisions: %d\n", total_slots, total_success, total_collisions);
    printf("Throughput: %.2f%%\n", ((double)total_success / total_slots) * 100.0);
}

// Menu-driven main
int main() {
    srand((unsigned int)time(NULL));
    int choice;

    while (1) {
        printf("\n=============================================================\n");
        printf("    SIMULATION OF MAC PROTOCOLS: ALOHA, CSMA/CD, CSMA/CA     \n");
        printf("=============================================================\n");
        printf("  1. Pure ALOHA Simulation\n");
        printf("  2. Slotted ALOHA Simulation\n");
        printf("  3. CSMA/CD Simulation\n");
        printf("  4. CSMA/CA Simulation\n");
        printf("  5. Run All Simulations\n");
        printf("  0. Exit\n");
        printf("-------------------------------------------------------------\n");
        printf("Enter your choice: ");

        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n'); // clear invalid input
            printf("Invalid input. Please enter a number.\n");
            continue;
        }

        switch (choice) {
            case 1:
                simulate_pure_aloha(TOTAL_SLOTS);
                break;
            case 2:
                simulate_slotted_aloha(TOTAL_SLOTS);
                break;
            case 3:
                simulate_csma_cd(TOTAL_SLOTS);
                break;
            case 4:
                simulate_csma_ca(TOTAL_SLOTS);
                break;
            case 5:
                simulate_pure_aloha(TOTAL_SLOTS);
                simulate_slotted_aloha(TOTAL_SLOTS);
                simulate_csma_cd(TOTAL_SLOTS);
                simulate_csma_ca(TOTAL_SLOTS);
                break;
            case 0:
                printf("Exiting simulation. Goodbye!\n");
                return 0;
            default:
                printf("Invalid choice. Please select 0-5.\n");
        }
    }

    return 0;
}