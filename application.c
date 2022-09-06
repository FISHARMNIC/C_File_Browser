#include <gtk/gtk.h>
#include "system.c"

GtkWidget *window;
GtkWidget *ParentVStack;
GtkWidget *VerticalStack;
GtkWidget *TopRow;
GtkWidget *addressBar;
GtkWidget *Scrollable;
GtkEntryBuffer *AddrBuffer;

#define MAX_FILES 1000
#define MAX_DIR_LEN 300

char *lsBuffer[MAX_FILES];
GtkWidget *lsButtons[MAX_FILES];
int lsID[MAX_FILES];
int numFilesInDir = 0;

char *filesOpenWith = "TextEdit";

extern int commandToBuffer(char *buffer[], char *command);
static void enter_clicked(GtkWidget *widget, gpointer data);
void renderElements();
char *Directory;

void clearVBox()
{
    for (int i = 0; i < numFilesInDir; i++)
        gtk_box_remove(GTK_BOX(VerticalStack), lsButtons[i]);
}

void checkNoHangingSlash()
{
    if (*(Directory + strlen(Directory) - 1) != '/')
        strcat(Directory, "/");
}

static void file_clicked(GtkWidget *widget, gpointer data)
{
    int ID = *((int *)data);
    char *oString = malloc(100 + MAX_DIR_LEN);
    checkNoHangingSlash();
    strcpy(oString, "open -a ");
    strcat(oString, filesOpenWith);
    strcat(oString, " ");
    strcat(oString, Directory);
    strcat(oString, lsBuffer[ID]);
    system(oString);
}

static void folder_clicked(GtkWidget *widget, gpointer data)
{
    int ID = *((int *)data);
    checkNoHangingSlash();
    strcat(Directory, lsBuffer[ID]);
    renderElements();
}

static void back_button_clicked(GtkWidget *widget, gpointer data)
{
    if (strcmp(Directory, "/") == 0 || *Directory == 0)
        return;
    char *dPointer = Directory + strlen(Directory) - 2;
    while (*(dPointer--) != '/')
        ;
    memset(dPointer + 2, 0, MAX_DIR_LEN - (dPointer - Directory) - 1);
    renderElements();
}

static void enter_clicked(GtkWidget *widget, gpointer data)
{
    char *AddressBuffer = (char *)gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(addressBar)));
    strcpy(Directory, AddressBuffer);

    renderElements();
}

static void OpenSelect(GtkWidget *widget, gpointer data)
{
    filesOpenWith = (char *)data;
}

static void activate(GtkApplication *app, gpointer user_data)
{
    /*
    VStack(
        Hstack(back, address, enter)
        Scrollable(
            VStack(...elemets)
        )
        BottomBar(
            ...opener
        )
    )

    */
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "File Viewer");
    gtk_window_set_default_size(GTK_WINDOW(window), 500, 300);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);

    // vertical stack for top being the search bar + enter button
    VerticalStack = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    ParentVStack = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    Scrollable = gtk_scrolled_window_new();

    TopRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 50);

    // search bar and enter button
    AddrBuffer = gtk_entry_buffer_new(Directory, -1);
    addressBar = gtk_entry_new_with_buffer(AddrBuffer);
    gtk_widget_set_size_request(addressBar, 500, 10);

    GtkWidget *addressButton = gtk_button_new_with_label("Enter");
    g_signal_connect(addressButton, "clicked", G_CALLBACK(enter_clicked), NULL);

    GtkWidget *BackButton = gtk_button_new_with_label("<- Back");
    g_signal_connect(BackButton, "clicked", G_CALLBACK(back_button_clicked), NULL);

    GtkWidget *BottomText = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(BottomText), g_markup_printf_escaped("<span size=\"x-large\">\%s</span>", "Open Files With"));
    GtkWidget *BottomBar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);

    GtkWidget *OW_TextEdit = gtk_button_new_with_label("TextEdit");
    GtkWidget *OW_Safari = gtk_button_new_with_label("Safari");
    GtkWidget *OW_Preview = gtk_button_new_with_label("Preview");

    g_signal_connect(OW_TextEdit, "clicked", G_CALLBACK(OpenSelect), (void *)"TextEdit");
    g_signal_connect(OW_Safari, "clicked", G_CALLBACK(OpenSelect), (void *)"Safari");
    g_signal_connect(OW_Preview, "clicked", G_CALLBACK(OpenSelect), (void *)"Preview");

    gtk_box_append(GTK_BOX(BottomBar), OW_TextEdit);
    gtk_box_append(GTK_BOX(BottomBar), OW_Safari);
    gtk_box_append(GTK_BOX(BottomBar), OW_Preview);

    // Top bar
    gtk_box_append(GTK_BOX(TopRow), BackButton);
    gtk_box_append(GTK_BOX(TopRow), addressBar);
    gtk_box_append(GTK_BOX(TopRow), addressButton);

    gtk_box_append(GTK_BOX(ParentVStack), TopRow);

    gtk_box_append(GTK_BOX(ParentVStack), Scrollable);
    gtk_box_append(GTK_BOX(ParentVStack), BottomText);
    gtk_box_append(GTK_BOX(ParentVStack), BottomBar);

    gtk_widget_set_size_request(Scrollable, 500, 500);
    gtk_widget_set_size_request(BottomBar, 500, 10);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(Scrollable), VerticalStack);

    gtk_window_set_child(GTK_WINDOW(window), ParentVStack);
    gtk_window_present(GTK_WINDOW(window));

    renderElements();
}

void renderElements()
{
    gtk_entry_buffer_set_text(AddrBuffer, Directory, -1);

    clearVBox();
    char *fullCommmand = malloc(MAX_LINE_LENGTH);

    strcpy(fullCommmand, "if cd \"");
    strcat(fullCommmand, Directory);
    strcat(fullCommmand, "\"\n then \n ls -t -p \n fi");

    g_print("Rendering With: %s\n", fullCommmand);
    numFilesInDir = commandToBuffer(lsBuffer, fullCommmand);
    free(fullCommmand);

    if (numFilesInDir == 0)
    {
        numFilesInDir++;
        GtkWidget *txt = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(txt), g_markup_printf_escaped("<span size=\"xx-large\">\%s</span>", "folder does not exist or is empty"));
        lsButtons[0] = txt;
        gtk_box_append(GTK_BOX(VerticalStack), txt);
    }
    else
    {
        for (int i = 0; i < numFilesInDir; i++)
        {
            GtkWidget *button = gtk_button_new();
            GtkWidget *txt = gtk_label_new(NULL);
            gtk_label_set_markup(GTK_LABEL(txt), g_markup_printf_escaped("<span size=\"xx-large\">\%s</span>", lsBuffer[i]));
            gtk_button_set_child(GTK_BUTTON(button), txt);

            lsButtons[i] = button;
            lsID[i] = i;
            // if last char is a '/' meaning its a folder
            if (lsBuffer[i][strlen(lsBuffer[i]) - 1] == '/')
            {
                g_signal_connect(button, "clicked", G_CALLBACK(folder_clicked), &lsID[i]);
            }
            else
            {
                g_signal_connect(button, "clicked", G_CALLBACK(file_clicked), &lsID[i]);
            }
            gtk_box_append(GTK_BOX(VerticalStack), button);
        }
    }
}

int main(int argc, char **argv)
{
    GtkApplication *app;
    int status;

    Directory = malloc(MAX_DIR_LEN);
    strcpy(Directory, "/");

    app = gtk_application_new("org.gtk.example", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}