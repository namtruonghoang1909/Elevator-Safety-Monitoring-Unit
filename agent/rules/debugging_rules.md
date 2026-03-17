### ------------------ DEBUGGING RULES ------------------ ###
### DEBUG
- if any API we use returns a esp_err_t, we add a log code below that to check the return esp_err_t 
- any critial code sections should have ESP_LOGI to spectate the code behavior.

### FIX BUG
- when an error detected:
    - report to the developer where the error is at.
    - tell the developer the cause of the bug.
    - report a plan to fix the bug
    - fix the bug then attempt to rebuild the project
    - keeps repeating the process until the bug has been resolved
- after successfully fixed a bug, store the bug info and the solution to memory\fixed_bugs.md 