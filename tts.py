from transformers import pipeline
import sounddevice as sd
import numpy as np

synthesiser = pipeline("text-to-speech", "suno/bark-small")

speech = synthesiser("Hello, my dog is cooler than you!", forward_params={"do_sample": True})

audio_data = np.frombuffer(speech["audio"], dtype=np.int16)
audio_normalized = audio_data.astype(np.float32) / np.iinfo(np.int16).max

sd.play(audio_normalized, speech["sampling_rate"])
sd.wait()  