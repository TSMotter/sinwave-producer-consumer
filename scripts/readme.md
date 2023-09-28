# stream_sin_wave.py

- This script presents a generic way to generate samples of a sinusoidal wave
- The samples can be generated and also streamed out in a variety of ways which can be selected based on the `--operation` parameter

## Channel's name fields
- A channel name follows the pattern: `channel_X_Y_Z`, where
    - `X`: is the number of samples present in each sinusoidal cycle
        - The larger this number is, the larger is the sample rate and thus we have a more faithful and smooth representation of the real curve (Nyquist–Shannon sampling theorem)

    - `Y`: represents the time step (in milliseconds) between 2 subsequent batches of data.
        - Data is sent in json format
        - Each "batch" streamed by the websocket contains one or more samples
        - If `Y` is set to 20, each new batch of data will be within 20 milli seconds from the previous one
        - The amount of samples within that batch depends on the values of `X` and `Z`
        - The larger this number is, more data will be received at once and the time without receiving new data will also be larger
            - Example: if you specify `Y=10000` you'll only receive 1 batch of data each 10 seconds, and the socket will be silent for around 10 seconds also

    - `Z`: is the frequency of the sinusoidal wave itself

### Example
- If you specify `channel_20_2000_1`
    - You'll have `X=20` samples in each wave cycle
    - You'll have one new batch of data being streamed every **2 seconds**, because of `Y=2000`
    - You'll have a sinusoidal wave with frequency of 1Hz because of `Z=1`
        - Which means the period of the wave is **1 second** (`T = 1/1`)
    - This means that each new batch received will contain samples that represent 2 full wave cycles, which means 40 samples/batch
