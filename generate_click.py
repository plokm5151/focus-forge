import wave, struct, math

sample_rate = 44100
duration = 0.05
frequency = 1000

obj = wave.open('assets/audio/click.wav', 'w')
obj.setnchannels(1)
obj.setsampwidth(2)
obj.setframerate(sample_rate)

for i in range(int(sample_rate * duration)):
    # Simple decaying sine wave
    envelope = math.exp(-i / (sample_rate * 0.01))
    value = int(32767.0 * envelope * math.sin(2.0 * math.pi * frequency * i / sample_rate))
    data = struct.pack('<h', value)
    obj.writeframesraw(data)

obj.close()
print("click.wav generated.")
