# Lightweight Chess Engine

A chess engine written in C++ 

## Overview

This chess engine implements a variety of chess programming concepts while maintaining a simple to read codebase. 

## Features

### Board Representation & Move Generation
- Bitboard-based board representation using 64-bit integers
- Magic Bitboards for sliding pieces (bishops, rooks, queens)
- Pre-computed attack tables for leaping pieces
- Special move handling (castling, en-passant, promotions)

### Search & Evaluation
- Alpha-Beta search with enhancements (LMR, PVS, Quiescence)
- Material and positional scoring
- Move ordering with MVV-LVA, killer moves, and history heuristics

### Optimizations
- Transposition Table with Zobrist hashing
- XORSHIFT32 for fast pseudo-random number generation
- Position caching and move ordering
- Built-in PERFT testing for movegen validation

### Interface
- UCI protocol support for chess GUI compatibility
- Time management system
- Iterative deepening search

## Todo

- Neural network evaluation (NNUE)

## Acknowledgments

- This engine is based on Code Monkey king's [Bitboard Chess engine](https://github.com/maksimKorzh/bbc), 
An amazing project which makes chess engine programming accesible for all!