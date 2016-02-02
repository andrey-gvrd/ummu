## Micro-MMU

This is a very basic memory management unit for flash memories around a MB used mostly in embedded systems.

This MMU was written to save a specific kind of data -- objects > 256B, < 63KB made up of a user-identified header and uniform data. It is heavily tied to the design of flash memory and the API exposed by flash manufacturers.

And example would be a series of measurements. In this case header holds a timestamp, a type of measurement and some kind of ID and data is saved as a byte-array.

### Interface

<pre>
                ┌──────────┐                        ┌──────────┐
                │          │                        │          │
 --->uMMU_Save()│          │---------->Memory_Save()│          │
                │   uMMU   │<----------Memory_Load()│  Memory  │
 <---uMMU_Load()│          │-->Memory_Sector_Erase()│          │
                │          │                        │          │
                └──────────┘                        └──────────┘
</pre>

#### uMMU_Save(type, out_buffer)

> Type - BT_Header:
> Initiates a new object.

> Type - BT_Data:
> Appends data to an existing object.

#### uMMU_Load(type, in_buffer)

> Type - BT_Header:
> Shifts pointer to the next object, returns it's data chunk. Erases previous object.

> Type - BT_Data:
> Returns current object's data chunk.

An example of use:

<pre>
      ┌─Object 1────┐
      │┌──────┐  ┌┐ │
      ││Header│->││ │
      │└──────┘  └┘ │
      └─────────────┘

      ┌─Object 2───────────────────┐
      │┌──────┐  ┌──────────┬─────┐│
─────>││Header│->│Data      ┆     ││
   ┆  │└──────┘  └──────────┴──^──┘│
   ┆  └────────────────────────┆───┘
-----uMMU_Save(BT_Data, …)-----┘
   ┆
<-uMMU_Load(BT_Header, …)
   ┆
   ┆  ┌─Object 3───────────────────┐
   v  │┌──────┐  ┌──────────┬─────┐│
─────>││Header│->│Data      ┆     ││
      │└──────┘  └──────────┴──^──┘│
      └────────────────────────┆───┘
--uMMU_Save(BT_Data, …)--------┘
</pre>

### Usage

To use this library you will need to:

1. Supply your own low-level functions for your memory IC:

- Memory_Save
- Memory_Load
- Memory_Sector_Erase

2. Specify parameters of your memory in the header.

Or use the ones I wrote for [M25P80][1].

[1]: https://github.com/andrey-gvrd/m25p80
