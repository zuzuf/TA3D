# Test for type parameters on both classes and methods
program TypeParameters;

class ListItem<T>
{
public
    # Constructor with item value
    method create(value) { item := value; }

    next: ListItem<T> := nil;
    item: T;
}

class List<T>
{
public
    # Empty constructor
    method create {}

    # Get the nth item or nil if there are not enough items
    method get(n)
    {
        if head = nil then return nil;

        current := head;
        while n > 0 and current != nil do
        {
            current := current.next;
            n--
        };
        current
    }

    # Add an item to the beginning of the list
    method prepend(item)
    {
        queue := if head = nil then nil else head.next;
        head := new ListItem<T>(item);
        head.next := queue
    }

    # Test method type parameters
    method addToEach<ValType>(value)
    {
    }

private
    head: ref ListItem<T> := nil;
}


# Test function type parameters
function printList<ListType>(list: ListType)
{
    foreach item in list do
        println("\t" << item)
}


function main
{
    l = List<int>.create;
    l.prepend(24);
    l.prepend(12);
    l.addToEach(2);
    printList(l);
    l.get(1)
}
