// Copyright (c) 2020 Trevor Taylor
//
// Permission to use, copy, modify, distribute and sell this software
// and its documentation for any purpose is hereby granted without fee,
// provided that the above copyright notice appear in all.
// Trevor Taylor makes no representations about the suitability of this
// software for any purpose.  It is provided "as is" without express or
// implied warranty.
//

pub struct Disposition
{
    pub select: bool,
    pub recurse: bool
}

pub trait SelectByValue<T> {
    fn select_by_value(&self, value: &T) -> bool;
}


pub struct Node<T>
{
    pub value : T,
    pub children : Vec<Node<T> >,
}

// selected set of sub-trees of a Node<T> that allows
// tree mutation
pub struct MutableSelection<T>
{
    root : &'a mut Node<'a,T>,
    // paths from root of selected sub-trees, each being a non-empty
    // list of indices capturing the path from root to the selected node
    selected_paths : Vec<Vec<usize> >
}

impl<'a, T, F1> MutableSelection<'a, T>
{
    // select descendants of root per selector
    pub fn by_value<F1>(root : &'a mut Node<T>,
			selector: F) -> MutableSelection<'a, T>
    where
	F1: Fn(&T) -> bool
    {
	let mut selected_paths : Vec<Vec<usize>> = {}
	for i in [0..root.children.size()] {
	    selected_paths.extend(select_by_value(root,i,selector));
	}
	MutableSelection<'a, T>{ root, selected_paths }
    }

    pub get_selected_values(self : &MutableSelection) -> Vec<T>
    {
	let result : Vec<T> = vec![];
	for p in &self.selected_paths {
	    result.push(self.resolve(p));
	}
    }

    fn get_value(self : &MutableSelection, &Vec<usize> p) -> T
    {
	let mut result : &Node<T> = self.root;
	for i in &p {
	    result = &result.children[*i];
	}
	return result.value;
    }
}

// get paths from parent of nodes from the subtree parent.children[index]
// that are selected by selector
fn select_by_value<T, F1>(
    parent : & Node<T>,
    index : usize,
    F1 selector) -> Vec<Vec<usize>>
where
    F1: Fn(&T) -> bool
{
    let node = &parent.children[i];
    let path_to_node : Vec<usize> = vec![ index ];
    let mut result : Vec<Vec<usize>>{};
    if selector(node.value){
	result.push( path_to_node );
    }
    for i in [0..node.children.size()] {
	let selected_children = select_by_value(node, i, selector);
	for c in selected_children {
	    c.extend(path_to_node.iter());
	    result.push(c);
	}
    }
    return result;
}


    pub struct Path<'a,T>
    {
	pub root : &'a mut Node<T>,
	pub indices_from_root: Vec<usize>
    }

    impl<'a,T> Path<'a,T>
    {
	pub fn target(self : &'a mut Path<'a,T>) -> &'a mut Node<T>
	{
	}
    }
	    
    pub impl<'a> Node<'a,T>
    {
	fn value(self : &'a Node<'a,T>) -> &'a T { self.value }
	fn value(self : &'a mut Node<'a,T>) -> &'a mut T { self.value }
	fn select_children(self : &'a mut Node<'a,T>,
			   selector : &SelectByValue<T>) -> Vec<'a,Path>
	{
	    let result : Vec<'a,Path> = new Vec<'a, Path>{};
	    for(i, child : enumerate(self.children) )
	    {
		let disposition = selector(child.value);
		if (disposition.select){
		    result.push(Path{
			root: self,
			indices_from_root : [i] } );
		}
		if (disposition.recurse){
		    let child_paths = child.select_children(selector);
		    for p in child_paths {
			result.push_back(Path{
			    root: self,
			    indices_from_root: [i]+p.indices_from_root } );
		    }
		}
	    }
	    return result;
	}
    }
    
    impl<'a, T> MutableSelection<'a, T>
    {
	// selects tree's root (if any)
	fn new(tree : &'a mut Tree,
	       selector: &SelectByValue<T>) -> MutableSelection<'a, T> {
	    MutableSelection { tree: tree, selected_paths : Vec<Path>{} }
	}

	// narrow to just nodes/children of nodes of self selected by selector
	fn narrow(self : &'a mut MutableSelection<'a, T>,
		  selector: &SelectByValue<T>) -> MutableSelection<'a, T>
	{
	    let narrowed : Vec<Path> = {}
	    for(p in self.selected_paths)
	    {
		let relative_paths = self.tree.find(p).select(selector);
		for(r in relative_paths)
		{
		    narrowed.extend(p + r);
		}
	    }
	    return MutableSelection(self.tree, narrowed);
	}

	// REVISIT: selection narrow(selection&& selection, select_by_path const& s);

	// remove selection, returns liberated nodes (branches)
	// (note nested matches remain in place in returned nodes)
	fn prune(self : &'a mut MutableSelection<'a, T>) -> Vec<Node<T>>
	{
	    // REVISIT
	}

	// pre: no selection or const_selection references tree
	// selection select(tree& tree, select_by_path const& s);

	// place copies of nodes before each node (non-nested) of selection
	//selection precede(selection&& selection, children_type nodes);

	// replace each selected node (non-nested) with copy of nodes
	//selection replace(selection&& selection, children_type nodes);

	//class values{
	//explicit values(tree& tree) noexcept: tree_(tree) {}

	//std::ref<tree> tree_;
	//std::vector<std::ref<value_type> > selected_values_;

	// values of all nodes in selection including/excluding descendents
	fn values(self : & MutableSelection<'a, T>) -> Vec<T>
	{
	    // REVISIT
	    Vec<T>{}
	}

	// REVISIT: mutable references version of values
*/

    // convenience wrappers

  // direct children of tree
  // pre: no existing const_selection/selection/values refer to tree
    //selection children_of(tree& tree);
  //selection children_of(selection&& selection);
  
  // calls f on each root node, calling f on result (advanced to next
  // valid node) until end is returned, where f's signature is:
  //   const_path f(node const& n, const_path path_to_n, const_path const& end)
  // notes:
  // - returning a parent of n is prohibited
  // - where f returns end-of-siblings, next() is applied before again
  //   calling f
  // - where f returns path unmodified, next() is applied before again
  //   calling f
  // pre: no exsiting const_selection, selection or values refers to tree
  // void walk(std::function<const_path, node const&, const_path, const_path const&> f) const;
  // void walk(std::function<path, node&, path, path const&> f);

  // static bool has_value(const_path const& p) {
  //   return p.first.size() && p.back().first.end() != p.back.second;
  // }
  // static bool end_of_siblings(const_path const& p) {
  //   return !is_empty(p) && p.back().second != p.back().first.end(); }
  // static bool has_children(const_path const& p) {
  //   return p.back().second->children_.size(); }

  
  // // conveniences for implementing f for apply()
  // // pre: pre: p != end
  // // post: has_value(result)
  // static const_path parent_of(const_path p, const_path const& end) noexcept;
  // // pre: !end_of_siblings(p)
  // static const_path next_sibling_of(const_path p) noexcept;
  // static path       next_sibling_of(path p) noexcept;
  // // pre: has_children(p)
  // static const_path first_child_of(const_path p) noexcept;
  // static path first_child_of(path p) noexcept;

  // // depth first order
  // // pre: p != end
  // // post: has_value(result) || result == end
  // static const_path next(const_path p, const_path const& end) noexcept
  // {
  //   if (has_children(p)){
  //     return first_child_of(std::move(p));
  //   }
  //   if (!end_of_siblings(p)){
  //     p = next_sibling_of(p);
  //   }
  //   while (p != end && !has_value(p)){
  //     p = next_sibling_of(parent_of(p, end));
  //   }
  //   return next(parent_of(p, end), end);
  // }
//}

// Template<class value_type, class tag=void>
// bool tree<value_type, tag>::selects_some(
//   tree<value_type, tag>::const_selection const& candidates,
//   tree<value_type, tag>::select_by_value const& s) const
// {
//   auto const d(p(value_));
//   switch(d){
//   case yes:
//   case yes_recurse:
//     return true;
//   case no_recurse:
//     for(auto node: children_){
//       if (node.has_some(p)){
//         return true;
//       }
//     }
//     break;
//   case no:
//     break;
//   default:
//     xju::assert_never_reached();
//   }
//   return false;    
// }


// }


