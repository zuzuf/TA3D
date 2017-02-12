Notes to developers - General guidelines
========================================

- Keep your code as clean and documented as possible, even (especially)
  in a work in progress
- The public API must be as simple as possible, with as few
  parameters as possible for usual functions
- The public API must not depend on any external library
- The public API must work seamlessly on every supported
  platform. There might be a few exceptions but there must be a
  __very good reason__ for it.
- Think low-level, write high-level.
   Keep in mind these important points :
   . Code maintenance
   . Code execution speed
   . Code simplicity
   . Portability
   . Code elegance and high-level characteristics

