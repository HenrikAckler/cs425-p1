## Task 1: Source Prep

I trimmed and added a markdown file `assignment.md` with the canvas assignment. I removed the things I don't want the AI worrying about. 

I also added the entire RFC 5321 doc to a markdown, then gave AI the following prompt:
```
I want you to look at this document.
Then, based on the information in assignment.md, I want you to strip out all information that is not needed to complete the assignment. 
For example, the entire section 1 is largely useless to you, so it would be stripped out. 
Most of section 2 can be simplified, and the original referenced in detail later as needed.
Section 4.1 and 4.2 is rather valuable, the rest of section 4 less so.
```
I reviewed the output, and I was fairly happy with how it trimmed down the document. It seemed to be getting the information I wanted to keep fairly well. I'll need to help it out more later, but this will do. 


## Task 2: Planning

I kicked it off with this prompt:

```
We're going to start some planning. For reference, please review the files in sourceForAI.

All code for now will be in src.

I'd like to start by planning out what files we're going to have. I believe we should have the following:

main.c - responsible for handling the command line input, setting things up and calling session
protocHelpers.[h/c] - this should be responsible for taking text strings and return codes, and appropriately handling them (converting into data, setting return values, etc). This is not going to touch any IO, this simple takes the text behind SMTP and understands it.
session.[h/c] - this should handle the active session. It will use a provided socket transport class to handle all I/O, and then use the protocHelper class to process all data. It is basically our logic layer.
socketTransport.[h/c] - This is a wrapper class on top of the sockets that we can swap out.

Look through steps 1 and 2 of the assignment especially, and tell me what you think of this plan. 

Keep in mind, the current main.c, lab.c/h, and test file implemtnation is junk, don't worry about it. We'll replace it.
```

It gave a pretty good response, largely recorded in `implementation-plan.md`. I'm going to use this a bit differently than it suggests, however. The goal is to get to a minimum viable product pretty fast. So first, I'm going to have it make the empty files it needs. Then we're going to get main working, then start stretching for some proper working code. 

## Task 3: Implementation

I kicked it off by first working with the AI to add the files that were planned, just as stubs. Then it was time for working on command line parsing in `main.c`, per step 5. 