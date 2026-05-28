# AutoSnake

Un jeu Snake où un agent IA apprend à jouer par Deep Q-Learning, codé en C++.

## Techniques utilisées

- **Double DQN** : deux réseaux séparés pour choisir et évaluer les actions, réduit la surestimation des Q-values
- **Dueling Networks** : le réseau sépare la valeur de l'état et l'avantage par action (haut, bas, droite, gauche)
- **Prioritized Experience Replay** : les expériences où le réseau s'est le plus trompé sont rejouées en priorité

## Architecture du réseau

Entrée : 15 valeurs (dangers relatifs, dangers absolus, direction, position de la nourriture)  
Couches partagées : 15 → 128 → 128  
Tête valeur : 128 → 64 → 1  
Tête avantage : 128 → 64 → 4  

## Entraînement

~1000 parties, scores atteignant 20–40. L'agent apprend à naviguer vers la nourriture en évitant les murs et son propre corps. Les performances sont limitées par la représentation locale à 15 valeurs, des algorithmes de pathfinding (vision globale) permettrait d'aller plus loin.
