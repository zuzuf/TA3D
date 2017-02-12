# Test containing a workflow
program document;

workflow DocumentWorkflow
{
states
	default intro;
	#! Introduction
	state approved;
	state archived;
	state created;
	state deleted;
	state updated;
	state waitingForApproval;
	state rejected;

transitions
	default forbid;
	allow waitingForApproval => approved;
	allow created => updated;
	allow approved, updated, rejected, deleted => updated;
	allow updated => waitingForApproval;
	allow approved => archived;
	allow *, -archived => rejected, deleted;
}

class Document
{
published
   #! A read/write property, which is a workflow
   property status: DocumentWorkflow;
   #! Title of the document
   property title;
   #! Author of the document
   property author;
   #! Summary
   property summary;
   #! Content
   property content;
   #! Date of creation
   property creationDate read pCreationDate;
   #! Timestamp of the last update
   property lastUpdate read pLastUpdate;

protected
   #! Timestamp of the last update
   pLastUpdate: timestamp;
   #! Date of creation
   pCreationDate: timestamp;
}

function main
{
    x := & Fibonacci(50) ^ Fibonacci(10);
	countdown := 5;
	timeout 1s do
	{
		// we wait here for the asynchronous result of x
		println("Fibonacci(50) ^ Fibonacci(10) = ", x);
	}
	else
	{
		if --countdown then
		{
			system.cout << countdown << '\n';
			// continue the execution of the expression and wait for 1s again
			continue;
		}
		else
		{
			abort(x);
			system.cout << "Operation timeout\n";
			return -1;
		}
	}
	return 0;
}
