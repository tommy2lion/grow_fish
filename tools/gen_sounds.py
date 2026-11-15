#!/usr/bin/env python3
"""Generate original CC0 sound effects for grow_fish.

Outputs 16-bit mono WAV files to assets/sounds/.
No third-party samples or libraries are used.
"""
import math
import os
import random
import struct
import wave

RATE = 22050
OUT = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
                   "assets", "sounds")


def clamp(x):
    return max(-1.0, min(1.0, float(x)))


def write_wav(name, samples):
    path = os.path.join(OUT, name)
    with wave.open(path, "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(RATE)
        data = bytearray()
        for s in samples:
            data += struct.pack("<h", int(clamp(s) * 32767))
        w.writeframes(data)
    print(f"{name:12s} {len(samples) / RATE:5.2f} s")


def env(i, n, attack=0.01, release=0.10):
    t = i / RATE
    a = min(1.0, t / attack) if attack > 0 else 1.0
    r = min(1.0, (n - i) / (release * RATE)) if release > 0 else 1.0
    return a * r


def tone(freq, dur, vol=0.5, decay=4.0, harmonics=(1.0,)):
    n = int(RATE * dur)
    out = []
    for i in range(n):
        t = i / RATE
        e = math.exp(-decay * t) * env(i, n)
        s = sum(h * math.sin(2 * math.pi * freq * k * t)
                for k, h in enumerate(harmonics, start=1))
        out.append(vol * e * s)
    return out


def noise_burst(dur, vol=0.6, decay=8.0, lowpass=0.0):
    n = int(RATE * dur)
    out = []
    low = 0.0
    for i in range(n):
        t = i / RATE
        x = random.uniform(-1.0, 1.0)
        if lowpass > 0:
            low = low * lowpass + x * (1.0 - lowpass)
            x = low
        out.append(vol * x * math.exp(-decay * t) * env(i, n))
    return out


def mix(*tracks):
    n = max(len(t) for t in tracks)
    out = [0.0] * n
    for t in tracks:
        for i, v in enumerate(t):
            out[i] += v
    return out


def sequence(notes, dur, vol=0.45, decay=3.0):
    out = []
    for f in notes:
        out += tone(f, dur, vol, decay, (1.0, 0.4, 0.15))
    return out


def main():
    random.seed(20261115)
    os.makedirs(OUT, exist_ok=True)

    # --- 水下环境音（8 秒循环） ---
    n = RATE * 8
    ambient = []
    low = 0.0
    for i in range(n):
        t = i / RATE
        low = low * 0.995 + random.uniform(-1.0, 1.0) * 0.03
        swell = 0.55 + 0.45 * math.sin(2 * math.pi * 0.07 * t + math.sin(2 * math.pi * 0.013 * t))
        ambient.append(low * 0.16 * swell)
    for _ in range(10):
        start = random.randint(0, n - 1)
        f = random.uniform(500, 1200)
        dur = random.uniform(0.08, 0.18)
        b = tone(f, dur, 0.05, 6.0)
        for j, v in enumerate(b):
            if start + j < n:
                ambient[start + j] += v
    fade = int(0.4 * RATE)
    for i in range(fade):
        ambient[i] *= i / fade
        ambient[n - 1 - i] *= i / fade
    write_wav("ambient.wav", ambient)

    # --- 短音效 ---
    write_wav("eat.wav", mix(
        noise_burst(0.12, 0.5, 18, 0.2),
        tone(500, 0.16, 0.4, 14, (1.0, 0.3))
    ))
    write_wav("hit.wav", mix(
        tone(90, 0.35, 0.7, 7, (1.0, 0.5)),
        noise_burst(0.25, 0.35, 12, 0.1)
    ))
    write_wav("bomb.wav", mix(
        noise_burst(0.7, 0.8, 5, 0.6),
        tone(55, 0.8, 0.6, 4, (1.0, 0.6))
    ))
    write_wav("clock.wav", sequence([880.0, 1174.7, 1568.0], 0.16, 0.35, 5.0))
    write_wav("win.wav", sequence([523.25, 659.25, 783.99, 1046.5], 0.18, 0.45, 3.0))
    write_wav("lose.wav", sequence([392.0, 329.63, 261.63], 0.28, 0.40, 2.5))
    write_wav("click.wav", mix(
        noise_burst(0.04, 0.25, 30, 0.0),
        tone(1200, 0.05, 0.20, 25)
    ))

    # --- 鱼猛回头 whoosh ---
    n2 = int(RATE * 0.25)
    turn = []
    for i in range(n2):
        t = i / RATE
        e = math.sin(math.pi * i / n2) ** 2
        turn.append(0.35 * random.uniform(-1.0, 1.0) * e *
                    (0.6 + 0.4 * math.sin(2 * math.pi * (300 + 800 * t) * t)))
    write_wav("turn.wav", turn)


if __name__ == "__main__":
    main()
