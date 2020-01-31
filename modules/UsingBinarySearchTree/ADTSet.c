///////////////////////////////////////////////////////////
//
// Υλοποίηση του ADT Set μέσω Binary Search Tree (BST)
//
///////////////////////////////////////////////////////////

#include <stdlib.h>
#include <assert.h>

#include "ADTSet.h"


// Υλοποιούμε τον ADT Set μέσω BST, οπότε το struct set είναι ένα Δυαδικό Δέντρο Αναζήτησης.
struct set {
	SetNode root;				// η ρίζα, NULL αν είναι κενό δέντρο
	int size;					// μέγεθος, ώστε η set_size να είναι Ο(1)
	CompareFunc compare;		// η διάταξη
	DestroyFunc destroy_value;	// Συνάρτηση που καταστρέφει ένα στοιχείο του set
};

// Ενώ το struct set_node είναι κόμβος ενός Δυαδικού Δέντρου Αναζήτησης
struct set_node {
	SetNode parent;				// Πατέρας
	SetNode left, right;		// Παιδιά
	Pointer value;
};


// Παρατηρήσεις για τις node_* συναρτήσεις
// - είναι βοηθητικές (κρυφές από το χρήστη) και υλοποιούν διάφορες λειτουργίες πάνω σε κόμβους του BST.
// - είναι αναδρομικές, η αναδρομή είναι γενικά πολύ βοηθητική στα δέντρα.
// - όσες συναρτήσεις _τροποποιούν_ το δέντρο, ουσιαστικά ενεργούν στο _υποδέντρο_ με ρίζα τον κόμβο node, και επιστρέφουν τη νέα
//   ρίζα του υποδέντρου μετά την τροποποίηση. Η νέα ρίζα χρησιμοποιείται από την προηγούμενη αναδρομική κλήση.
//
// Οι set_* συναρτήσεις (πιο μετά στο αρχείο), υλοποιούν τις συναρτήσεις του ADT Set, και είναι απλές, καλώντας τις αντίστοιχες node_*.


// Δημιουργεί και επιστρέφει έναν κόμβο με τιμή value (χωρίς πατέρα/παιδιά)

SetNode node_create(Pointer value) {
	SetNode node = malloc(sizeof(*node));
	node->parent = NULL;
	node->left = NULL;
	node->right = NULL;
	node->value = value;
	return node;
}

// Επιστρέφει τον κόμβο με τιμή ίση με value στο υποδέντρο με ρίζα node, διαφορετικά NULL

SetNode node_find_equal(SetNode node, CompareFunc compare, Pointer value) {
	// κενό υποδέντρο, δεν υπάρχει η τιμή
	if(node == NULL)
		return NULL;
	
	// Το πού βρίσκεται ο κόμβος που ψάχνουμε εξαρτάται από τη διάταξη της τιμής
	// value σε σχέση με την τιμή του τρέχοντος κόμβο (node->value)
	//
	int compare_res = compare(value, node->value);			// αποθήκευση για να μην καλέσουμε την compare 2 φορές
	if(compare_res == 0)									// value ισοδύναμη της node->value, βρήκαμε τον κόμβο
		return node;
	else if(compare_res < 0)								// value < node->value, ο κόμβος που ψάχνουμε είναι στο αριστερό υποδέντρο
		return node_find_equal(node->left, compare, value);
	else													// value > node->value, ο κόμβος που ψάχνουμε είνια στο δεξιό υποδέντρο
		return node_find_equal(node->right, compare, value);
}

// Επιστρέφει τον μικρότερο κόμβο του υποδέντρου με ρίζα node

SetNode node_find_min(SetNode node) {
	return node != NULL && node->left != NULL
		? node_find_min(node->left)				// Υπάρχει αριστερό υποδέντρο, η μικρότερη τιμή βρίσκεται εκεί
		: node;									// Αλλιώς η μικρότερη τιμή είναι στο ίδιο το node
}

// Επιστρέφει τον μεγαλύτερο κόμβο του υποδέντρου με ρίζα node

SetNode node_find_max(SetNode node) {
	return node != NULL && node->right != NULL
		? node_find_max(node->right)			// Υπάρχει δεξί υποδέντρο, η μεγαλύτερη τιμή βρίσκεται εκεί
		: node;									// Αλλιώς η μεγαλύτερη τιμή είναι στο ίδιο το node
}

// Επιστρέφει τον προηγούμενο του node στη σειρά διάταξης, ή NULL αν ο node είναι ο μικρότερος όλου του δέντρου

SetNode node_find_previous(SetNode node, CompareFunc compare) {
	// Αν έχουμε αριστερό παιδί τότε όλο το αριστερό υποδέντρο είναι μικρότεροι κόμβοι.
	// Ο πρηγούμενος είναι ο μεγαλύτερος από αυτούς.
	if(node->left != NULL)
		return node_find_max(node->left);

	// Δεν έχουμε αριστερό παιδί, μπορεί όμως να υπάρχουν μικρότεροι κόμβοι σε άλλα σημεία του υποδέντρου.
	// Προχωράμε προς τη ρίζα, ψάχνοντας για κόμβο που να είναι _δεξί_ παιδί του πατέρα του, άρα ο πατέρας
	// είναι ο ακριβώς προγούμενός του.
	for(; node != NULL; node = node->parent)
		if(node->parent != NULL && node->parent->right == node)
			return node->parent;

	// φτάσαμε στη ρίζα ακολουθώντας μόνο _αριστερά_ links, άρα είμαστε ο αριστερότερος (μικρότερος) κόμβος όλου του δέντρου!
	return NULL;
}

// Επιστρέφει τον επόμενο του node στη σειρά διάταξης, ή NULL αν ο node είναι ο μεγαλύτερος όλου του δέντρου

SetNode node_find_next(SetNode node, CompareFunc compare) {
	// Αν έχουμε δεξί παιδί τότε όλο το δεξί υποδέντρο είναι μικρότεροι κόμβοι.
	// Ο επόμενος είναι ο μεγαλύτερος από αυτούς.
	if(node->right != NULL)
		return node_find_min(node->right);

	// Δεν έχουμε δεξί παιδί, μπορεί όμως να υπάρχουν μεγαλύτεροι κόμβοι σε άλλα σημεία του υποδέντρου.
	// Προχωράμε προς τη ρίζα, ψάχνοντας για κόμβο που να είναι _αριστερό_ παιδί του πατέρα του, άρα ο πατέρας
	// είναι ο ακριβώς επόμενός του.
	for(; node != NULL; node = node->parent)
		if(node->parent != NULL && node->parent->left == node)
			return node->parent;

	// φτάσαμε στη ρίζα ακολουθώντας μόνο _δεξιά_ links, άρα είμαστε ο δεξιότερος (μεγαλύτερος) κόμβος όλου του δέντρου!
	return NULL;
}

// Αν υπάρχει κόμβος με τιμή ισοδύναμη της value, αλλάζει την τιμή του σε value, διαφορετικά προσθέτει
// νέο κόμβο με τιμή value. Επιστρέφει τη νέα ρίζα του υποδέντρου, και θέτει το *inserted σε true
// αν έγινε προσθήκη, ή false αν έγινε ενημέρωση.

SetNode node_insert(SetNode node, CompareFunc compare, Pointer value, bool* inserted, Pointer* old_value) {
	// Αν το υποδέντρο είναι κενό, δημιουργούμε νέο κόμβο ο οποίος γίνεται ρίζα του υποδέντρου
	// Ο κόμβος για την ώρα δεν έχει πατέρα, μπορεί να αποκτήσει αν το υποδέντρο τοποθετηθεί ως παιδί άλλου δέντρου.
	if(node == NULL) {
		*inserted = true;			// κάναμε προσθήκη
		return node_create(value);
	}

	// Το που θα γίνει η προσθήκη εξαρτάται από τη διάταξη της τιμής
	// value σε σχέση με την τιμή του τρέχοντος κόμβου (node->value)
	//
	int compare_res = compare(value, node->value);
	if(compare_res == 0) {
		// βρήκαμε ισοδύναμη τιμή, κάνουμε update
		*inserted = false;
		*old_value = node->value;
		node->value = value;

	} else if(compare_res < 0) {
		// value < node->value, συνεχίζουμε αριστερά. Η ρίζα του αριστερού υποδέντρου
		// ίσως αλλαξει, οπότε ενημερώνουμε το node->left με τη νέα ρίζα!
		node->left = node_insert(node->left, compare, value, inserted, old_value);
		node->left->parent = node;

	} else {
		// value > node->value, συνεχίζουμε αριστερά. Η ρίζα του δεξιού υποδέντρου
		// ίσως αλλαξει, οπότε ενημερώνουμε το node->right με τη νέα ρίζα!
		node->right = node_insert(node->right, compare, value, inserted, old_value);
		node->right->parent = node;
	}

	return node;	// η ρίζα του υποδέντρου δεν αλλάζει
}

// Αφαιρεί και αποθηκεύει στο min_node τον μικρότερο κόμβο του υποδέντρου με ρίζα node.
// Επιστρέφει τη νέα ρίζα του υποδέντρου.

SetNode node_remove_min(SetNode node, SetNode* min_node) {
	if(node->left == NULL) {
		// Δεν έχουμε αριστερό υποδέντρο, οπότε ο μικρότερος είναι ο ίδιος ο node
		*min_node = node;

		SetNode right = node->right;	// αποθήκευση πριν το free
		if(right != NULL)
			right->parent = node->parent;

		return right;					// νέα ρίζα είναι το δεξιό παιδί

	} else {
		// Εχουμε αριστερό υποδέντρο, οπότε η μικρότερη τιμή είναι εκεί. Συνεχίζουμε αναδρομικά
		// και ενημερώνουμε το node->left με τη νέα ρίζα του υποδέντρου.
		node->left = node_remove_min(node->left, min_node);
		return node;
	}
}

// Διαγράφει το κόμβο με τιμή ισοδύναμη της value, αν υπάρχει. Επιστρέφει τη νέα ρίζα του
// υποδέντρου, και θέτει το *removed σε true αν έγινε πραγματικά διαγραφή.

SetNode node_remove(SetNode node, CompareFunc compare, Pointer value, bool* removed, Pointer* old_value) {
	if(node == NULL) {
		*removed = false;		// κενό υποδέντρο, δεν υπάρχει η τιμή
		return NULL;
	}

	int compare_res = compare(value, node->value);
	if(compare_res == 0) {
		// Βρέθηκε ισοδύναμη τιμή στον node, οπότε τον διαγράφουμε. Το πώς θα γίνει αυτό εξαρτάται από το αν έχει παιδιά.
		*removed = true;
		*old_value = node->value;

		if(node->left == NULL) {
			// Δεν υπάρχει αριστερό υποδέντρο, οπότε διαγράφεται απλά ο κόμβος και νέα ρίζα μπαίνει το δεξί παιδί
			SetNode right = node->right;	// αποθήκευση πριν το free!
			if(right != NULL)
				right->parent = node->parent;

			free(node);
			return right;

		} else if(node->right == NULL) {
			// Δεν υπάρχει δεξί υποδέντρο, οπότε διαγράφεται απλά ο κόμβος και νέα ρίζα μπαίνει το αριστερό παιδί
			SetNode left = node->left;		// αποθήκευση πριν το free!
			if(left != NULL)
				left->parent = node->parent;

			free(node);
			return left;

		} else {
			// Υπάρχουν και τα δύο παιδιά. Αντικαθιστούμε την τιμή του node με την μικρότερη του δεξιού υποδέντρου, η οποία
			// αφαιρείται. Η συνάρτηση node_remove_min κάνει ακριβώς αυτή τη δουλειά.

			SetNode min_right;
			node->right = node_remove_min(node->right, &min_right);

			// Σύνδεση του min_right στη θέση του node
			min_right->parent = node->parent;
			min_right->left = node->left;
			min_right->right = node->right;

			free(node);
			return min_right;
		}
	}

	// compare_res != 0, συνεχίζουμε στο αριστερό ή δεξί υποδέντρο, η ρίζα δεν αλλάζει.
	if(compare_res < 0)
		node->left  = node_remove(node->left,  compare, value, removed, old_value);
	else
		node->right = node_remove(node->right, compare, value, removed, old_value);

	return node;
}

// Καταστρέφει όλο το υποδέντρο με ρίζα node

void node_destroy(SetNode node, DestroyFunc destroy_value) {
	if(node == NULL)
		return;
	
	// πρώτα destroy τα παιδιά, μετά free το node
	node_destroy(node->left, destroy_value);
	node_destroy(node->right, destroy_value);

	if(destroy_value != NULL)
		destroy_value(node->value);

	free(node);
}


//// Συναρτήσεις του ADT Set. Γενικά πολύ απλές, αφού καλούν τις αντίστοιχες node_*

Set set_create(CompareFunc compare, DestroyFunc destroy_value) {
	assert(compare != NULL);

	// δημιουργούμε το stuct
	Set set = malloc(sizeof(*set));
	set->root = NULL;			// κενό δέντρο
	set->size = 0;
	set->compare = compare;
	set->destroy_value = destroy_value;

	return set;
}

int set_size(Set set) {
	return set->size;
}

void set_insert(Set set, Pointer value) {
	bool inserted;
	Pointer old_value;
	set->root = node_insert(set->root, set->compare, value, &inserted, &old_value);

	// Το size αλλάζει μόνο αν μπει νέος κόμβος. Στα updates κάνουμε destroy την παλιά τιμή
	if(inserted)
		set->size++;
	else if(set->destroy_value != NULL)
		set->destroy_value(old_value);
}

Pointer set_remove(Set set, Pointer value) {
	bool removed;
	Pointer old_value = NULL;
	set->root = node_remove(set->root, set->compare, value, &removed, &old_value);

	// Το size αλλάζει μόνο αν πραγματικά αφαιρεθεί ένας κόμβος
	if(removed) {
		set->size--;

		if(set->destroy_value != NULL)
			set->destroy_value(old_value);
	}

	return old_value;
}

Pointer set_find(Set set, Pointer value) {
	SetNode node = node_find_equal(set->root, set->compare, value);
	return node == NULL ? NULL : node->value;
}

void set_destroy(Set set) {
	node_destroy(set->root, set->destroy_value);
	free(set);
}

SetNode set_first(Set set) {
	return node_find_min(set->root);
}

SetNode set_last(Set set) {
	return node_find_max(set->root);
}

SetNode set_previous(Set set, SetNode node) {
	return node_find_previous(node, set->compare);
}

SetNode set_next(Set set, SetNode node) {
	return node_find_next(node, set->compare);
}

Pointer set_node_value(Set set, SetNode node) {
	return node->value;
}

SetNode set_find_node(Set set, Pointer value) {
	return node_find_equal(set->root, set->compare, value);
}