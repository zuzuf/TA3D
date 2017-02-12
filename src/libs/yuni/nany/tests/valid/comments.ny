# This test checks for comment recognition
/*! Spreading doc comment
*/
program Comments;

# Simple comment

#! Doc comment
function main: int
{
    # Comment in function
    return 0; 	// Another line comment

    #* Spreading comment
    More commented stuff
    More and more ... and stop *#
}

	/* Another spreading comment
   Yee-haaaaa !
*/
