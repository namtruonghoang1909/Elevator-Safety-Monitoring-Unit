
### ------------------ CHAT RULES ------------------ ###
- Instead of typing in the chat, user might type in the agent/temporary/task.md, then agent will be told to read the task.md

## Extra [commands]
- When user have a [command] infront of a prompt, agent has to do some desired works. This basically to let the agent knows what its supposed to do.
    - [chat] → Conversation only. No file modifications or code execution unless explicitly requested.
    - [discuss] → Deep conceptual discussion with optional web research.
    - [quick] → Short, direct answer only.
    - [design] → High-level architecture or system design. Stores the answer to agent/plan/idea.md, continue writing to the file, do not overwrite.
    - [code] -> Agent about to implement a new function, feature.
    - [refactor] → Improve code structure and modularity. Show the new code to the chat section, waits for the approval of the user.
    - [comment] → Add or fix comments based on the context that user provides after the command.
    - [debug] → Analyze logs/errors and find root cause.
    - [fix] → This command usually run after the [debug] command, once the errors have been found, agent attempts to create todolist on the agent/plan/bug_fixing_plan.md,
    once the todolist was approved by the user, agent executes the todolist. continue writing to the file, do not overwrite.
    - [run] → This command usually followed by build, flash, test, serial monitor, agent will execute the commands based on the 
    provided commands in BUILD ACTIONS section then observe the output, report the thoughts about the output to the user.
    - [review] → Code or architecture critique.
    - [unit-test] → Generate proper unit tests (host-based if possible).
    - [integration-test] → Generate hardware-level or system tests.
    - [simulate_fails] → Simulate failure scenarios.
    - [doc] → Generate structured documentation.
    - [commit] → Generate commit / PR messages.
    - [resume] → Convert work into resume-ready bullet points.
    - [checkpoint] → This usually called before the user's quit. Modify the agent/context/checkpoint.md, stores things that the agent need to remember. 
    Next time the user opens the agent, agent will read agent/context/checkpoint.md to understand the previous working context. 
 