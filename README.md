Attempt on a dos4g/dos32a compatible s3m player

Mostly for nostalgic reasons

Compile with watcom on dosbox.

Does:
    - load s3m file, including patterns and samples
    - allows to play samples on fixed speeds (sb through DMA)
    - can "play" patterns in the correct orders (speed/tick works)

Still needs:
    - play samples on different frequencies (C4 vs F#6 etc)
    - mix all the channels into the main dma buffer for playing
    - add all effect commands
