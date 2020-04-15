# -*- coding: utf-8 -*-
from math import gcd
from typing import Tuple
from random import randint
import sys


def exgcd(a: int, b: int) -> Tuple[int, int]:
    # ax + by = \gcd(a, b)
    if b == 0:
        return 1, 0
    y, x = exgcd(b, a % b)
    return x, y - a // b * x


def inv(x: int, mod: int) -> int:
    return exgcd(x, mod)[0] % mod


def millerRabin(p: int) -> bool:
    def witness(v: int, p: int) -> bool:
        r, s, x = p - 1, 0, 0
        while ~r & 1:
            s += 1
            r >>= 1
        x = pow(v, r, p)
        if x == 1 or x == p - 1:
            return False
        while s:
            x = x * x % p
            if x == p - 1:
                return False
            s -= 1
        return True

    if p == 2:
        return True
    if p < 2 or not (p & 1):
        return False
    PRIME = [3, 5, 7, 11, 13, 17, 19, 23, 29]
    for i in PRIME:
        if p == i:
            return True
        if witness(i, p):
            return False
    for i in range(20):
        r = randint(2, p - 1)
        if witness(r, p):
            return False
    return True


class RSA:
    def __init__(self):
        self.e = self.d = self.phi = self.p = self.q = self.n = 0

    def generate(self, length: int) -> None:
        lower = 2 ** length
        upper = lower * 2 - 1
        while not millerRabin(self.p):
            self.p = randint(lower, upper)
        while self.q == self.p or not millerRabin(self.q):
            self.q = randint(lower, upper)
        self.n = self.p * self.q
        self.phi = (self.p - 1) * (self.q - 1)
        while gcd(self.phi, self.e) != 1:
            self.e = randint(2, self.phi)
        self.d = inv(self.e, self.phi)

    def encrypt(self, m: int, key=None) -> int:
        if key is None:
            key = (self.e, self.n)
        return pow(m, key[0], key[1])

    def decrypt(self, c: int, key=None) -> int:
        if key is None:
            key = (self.d, self.n)
        return pow(c, key[0], key[1])

    def getPrivateKey(self) -> Tuple[int, int]:
        return self.d, self.n

    def getPublicKey(self) -> Tuple[int, int]:
        return self.e, self.n


if __name__ == "__main__":
    sys.setrecursionlimit(10000000)
    d = RSA()
    d.generate(1024)
    print(f'private: {d.getPrivateKey()}\npublic: {d.getPublicKey()}')
    m = 998244353
    c = d.encrypt(998244353)
    print(d.encrypt(m))
    print(d.decrypt(c))
    assert d.decrypt(d.encrypt(m)) == m
    while True:
        m = int(input("please input a message"))
        if m <= 0:
            break
        print(d.encrypt(m))
        assert d.decrypt(d.encrypt(m)) == m
