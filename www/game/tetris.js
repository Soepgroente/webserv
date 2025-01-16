// block prototypes
const blocks = [
    {
        shape: [
            [0, 0, 0],
            [1, 1, 1],
            [0, 1, 0]
        ],
        position: [-1, 4],
        color_nr: 0
    },
    {
        shape: [
            [1, 1],
            [1, 1]
        ],
        position: [0, 4],
        color_nr: 1
    },
    {
        shape: [
            [0, 0, 0, 0],
            [1, 1, 1, 1],
            [0, 0, 0, 0],
            [0, 0, 0, 0]
        ],
        position: [-1, 3],
        color_nr: 2
    },
    {
        shape: [
            [1, 1, 0],
            [0, 1, 1],
            [0, 0, 0]
        ],
        position: [0, 4],
        color_nr: 3
    },
    {
        shape: [
            [0, 1, 1],
            [1, 1, 0],
            [0, 0, 0]
        ],
        position: [0, 4],
        color_nr: 4
    },
    {
        shape: [
            [1, 0, 0],
            [1, 1, 1],
            [0, 0, 0]
        ],
        position: [0, 4],
        color_nr: 5
    },
    {
        shape: [
            [0, 0, 1],
            [1, 1, 1],
            [0, 0, 0]
        ],
        position: [0, 4],
        color_nr: 6
    }
];

// constants
const nr_rows = 20;
const nr_columns = 10;
const colors = ["green", "blue", "red", "purple", "orange", "magenta", "cyan"];

// global variables
let tetris_table;
let drop_speed = 1000; // drop speed at level 0
let game_paused = true;
let game_over = false;
let high_score, score, level, total_lines, interval_id;

function tetris_run() {
    tetris_table = document.getElementById("tetris");
    // retrieve high score from memory
    try {
        high_score = localStorage.getItem("high_score");
        if (high_score == null) {
            high_score = 0;
        }
    } catch (error) {
        high_score = 0;
    }
    update_highscore(high_score);
    new_game();
}

function new_game() {
    document.getElementById("game-over").setAttribute("hidden", true);
    document.getElementById("high-score").innerHTML = high_score;

    // reset score
    add_score(0, true);
    new_level(0, true);
    total_lines = 0;

    // create the field
    field = [];
    for (let i = 0; i < nr_rows; i++) {
        field[i] = new Array(nr_columns);
    }
    // add field to DOM
    tetris_table.innerHTML = "";
    for (let i = 0; i < nr_rows; i++) {
        let newRow = tetris_table.insertRow(0);
        for (j = 0; j < nr_columns; j++) {
            newRow.insertCell(0);
        }
    }
    create_new_block();
    game_paused = false;
    game_over = false;
}

function create_new_block() {
    // create block from prototype (convert via JSON to create a deep clone)
    block = JSON.parse(JSON.stringify(blocks[Math.floor(Math.random() * blocks.length)]));
    print_field_to_DOM();

    // check if game over
    if (check_block(block.shape, block.position) === false) {
        // game over
        game_over = true;
        document.getElementById("game-over").removeAttribute("hidden");
    }
}

function print_field_to_DOM() {
    // print field to DOM
    for (let row = 0; row < nr_rows; row++) {
        let DOM_row = document.querySelectorAll("tr")[row];
        for (let column = 0; column < nr_columns; column++) {
            let DOM_block = DOM_row.querySelectorAll("td")[column];
            if (field[row][column] >= 0) {
                DOM_block.setAttribute("bgcolor", colors[field[row][column]]);
            } else {
                DOM_block.removeAttribute("bgcolor");
            }
        }
    }

    // print the partial blocks of the active Block to the DOM
    for (let i = 0; i < block.shape.length; i++) {
        let DOM_row = document.querySelectorAll("tr")[block.position[0] + i];
        for (let j = 0; j < block.shape[i].length; j++) {
            if (block.shape[i][j] === 1) {
                let DOM_block = DOM_row.querySelectorAll("td")[block.position[1] + j];
                DOM_block.setAttribute("bgcolor", colors[block.color_nr]);
            }
        }
    }
}

function move(delta_row, delta_column) {
    // check if move is legal
    if (check_block(block.shape, [block.position[0] + delta_row, block.position[1] + delta_column])) {
        // update position of the block
        block.position[0] = block.position[0] + delta_row;
        block.position[1] = block.position[1] + delta_column;
        // print new field
        print_field_to_DOM();
        return true;
    } else {
        return false;
    }
}

function long_drop() {
    let keep_going = true;
    while (keep_going) {
        keep_going = move(1, 0);
    }
    fix_block();
}

function rotate(direction) {
    // check if rotation is legal
    if (check_block(transpose(block.shape, direction), block.position)) {
        // update shape of the block
        block.shape = transpose(block.shape, direction);
        // print new field
        print_field_to_DOM();
    }
}

function transpose(matrix, direction) {
    // pure
    let transposed_shape = [];
    for (let i = 0; i < matrix[0].length; i++) {
        transposed_shape[i] = Array(matrix.length);
    }
    for (let i = 0; i < matrix.length; i++) {
        for (let j = 0; j < matrix[0].length; j++) {
            if (direction == 0) {
                // transpose clockwise:
                transposed_shape[j][matrix.length - 1 - i] = matrix[i][j];
            }
            if (direction == 1) {
                // transpose counter clockwise:
                transposed_shape[matrix[0].length - 1 - j][i] = matrix[i][j];
            }
        }
    }
    return transposed_shape;
}

function check_block(shape, position) {
    // loop over block elements, to check if they fit in the field
    for (let i = 0; i < shape.length; i++) {
        for (let j = 0; j < shape[i].length; j++) {
            if (shape[i][j] === 1) {
                if (validate_cell(position[0] + i, position[1] + j) === false) {
                    return false;
                }
            }
        }
    }
    return true;
}

function validate_cell(row, column) {
    if (row >= nr_rows || row < 0 || column >= nr_columns || column < 0) {
        // cell is outside the field
        return false;
    }
    if (field[row][column] != null) {
        // cell is already occupied
        return false;
    } else {
        // cell is available
        return true;
    }
}

function remove_full_rows() {
    let nr_full_rows = 0;
    for (let i = 0; i < nr_rows; i++) {
        if (validate_fullrow(i)) {
            remove_row(i);
            nr_full_rows++;
        }
    }
    return nr_full_rows;
}

function validate_fullrow(row) {
    for (let column = 0; column < nr_columns; column++) {
        if (field[row][column] == null) {
            return false;
        }
    }
    return true;
}

function remove_row(row) {
    // remove full row from field, and add a new one
    field.splice(row, 1);
    field.unshift(new Array(nr_columns));
}

function tick() {
    if (!game_paused && !game_over) {
        // if possible, drop block
        if (move(1, 0) == false) {
            fix_block();
        }
    }
}

function fix_block() {
    add_block_to_field();
    let nr_full_rows = remove_full_rows();
    total_lines = total_lines + nr_full_rows;
    let points = (1 + level / 10) * 100 * nr_full_rows ** 2;
    if (points > 0) {
        add_score(points);
    }
    create_new_block();
}

function add_block_to_field() {
    for (let i = 0; i < block.shape.length; i++) {
        for (let j = 0; j < block.shape[i].length; j++) {
            if (block.shape[i][j] === 1) {
                field[block.position[0] + i][block.position[1] + j] = block.color_nr;
            }
        }
    }
}

function add_score(new_points, reset) {
    if (reset) {
        score = 0;
    }
    score = score + new_points;
    document.getElementById("score").innerHTML = score;
    new_level(min_level());
    if (score > high_score) {
        update_highscore(score);
    }
}

function update_highscore(new_highscore) {
    high_score = new_highscore;
    localStorage.setItem("high_score", new_highscore);
    document.getElementById("high-score").innerHTML = new_highscore;
}

function new_level(new_level, force) {
    if (new_level > level || force) {
        level = new_level;
        document.getElementById("level").innerHTML = level;

        // drop the active block at the correct speed
        drop_speed = 1000 * Math.pow(0.75, level); // each level speed inscreases ~33%%
        if (interval_id) {
            window.clearInterval(interval_id);
        }
        interval_id = window.setInterval(tick, drop_speed);
    }
}

function min_level() {
    return Math.floor(total_lines / 5);
}

// event listeners:
window.addEventListener("keyup", function(e) {
    // console.log(e.keyCode);
    if (!game_paused && !game_over) {
        // left: 37
        if (e.keyCode === 37) {
            move(0, -1);
        }
        // right: 39
        if (e.keyCode === 39) {
            move(0, 1);
        }
        // down: 40
        if (e.keyCode === 40) {
            move(1, 0);
        }
        // q: 81
        if (e.keyCode === 81) {
            rotate(1);
        }
        // up: 38
        if (e.keyCode === 38) {
            rotate(0);
        }
        // space bar: 32
        if (e.keyCode === 32) {
            long_drop();
        }
        // p: 80
        if (e.keyCode === 80) {
            game_paused = true;
        }
        // numerical keys [0 - 9] have keyCodes [48 - 57]
        if (e.keyCode >= 48 && e.keyCode <= 57) {
            new_level(e.keyCode - 48);
        }
    } else {
        game_paused = false;
    }
    // n: 78
    if (game_over && e.keyCode === 78) {
        new_game();
    }
});
