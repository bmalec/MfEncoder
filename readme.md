MfEncoder
=========
A native, Media Foundation-based WMA audio encoder

Purpose
-------
This is a tutorial that demonstrates how to perform variable bitrate (VBR) audio compression with Media Foundation.  As far as I can tell, it is the only complete and functional example available on the internet that supports VBR... believe it or not. 

History
-------
For many years I've been using Exact Audio Copy to rip CDs I own, and since I essentially live in a Windows ecosystem I've had the need to compress audio files using the Windows Media Audio codec.

Originally I used Microsoft's Windows Media Encoder 9 command-line utility to perform this encoding, but eventually that product was discontinued and replaced with Microsoft Expression Encoder.  I couldn't figure out how to use Expression Encoder as a command line utility, but the Expression Encoder SDK was freely avaialble so I wrote a small console app in C#.  For the next few years it worked pretty well.

Then Expression Encoder was discontinued, so I started searching for yet another replacement for my compression needs.  It appeared that Media Foundation was the latest and greatest library available, so I set out to create a new utilty based on Media Foundation. 

