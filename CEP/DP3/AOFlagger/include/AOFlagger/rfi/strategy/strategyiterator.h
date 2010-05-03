#ifndef STRATEGY_ITERATOR_H
#define STARTEGY_ITERATOR_H

#include <stack>

#include <AOFlagger/rfi/strategy/strategy.h>

namespace rfiStrategy
{

	class StrategyIterator
	{
		public:
			StrategyIterator(const StrategyIterator &source)
				: _indices(source._indices), _currentAction(source._currentAction)
			{
			}

			StrategyIterator &operator++()
			{
				ActionContainer *container = dynamic_cast<ActionContainer*>(_currentAction);
				if(container != 0)
				{
					_currentAction = &container->GetFirstChild();
					_indices.push(0);
				} else {
					// Current action is not a container, so go back one level
					ActionContainer *parent = _currentAction->Parent();
					size_t index = _indices.top();
					_indices.pop();
					++index;

					while(index > parent->GetChildCount() && parent != 0)
					{
						index = _indices.top();
						_indices.pop();
						parent = parent->Parent();
						++index;
					}

					_indices.push(index);
					if(parent != 0 && index < parent->GetChildCount())
					{
						_currentAction = &parent->GetChild(index);
					} else {
						while(_currentAction->Parent() != 0)
							_currentAction = _currentAction->Parent();
					}
				}

				return *this;
			}
			Action &operator*() const
			{
				return *_currentAction;
			}
			Action *operator->() const
			{
				return _currentAction;
			}
	
			bool PastEnd() const
			{
				if(_currentAction->Parent() != 0)
				{
					return false;
				} else
				{
					ActionContainer *container = static_cast<ActionContainer*>(_currentAction);
					return _indices.top() >= container->GetChildCount();
				}
			}
		static StrategyIterator NewStartIterator(Strategy &strategy)
		{
			return StrategyIterator(&strategy, 0);
		}
		static StrategyIterator NewEndIterator(Strategy &strategy)
		{
			return StrategyIterator(&strategy, strategy.GetChildCount());
		}
		private:
			StrategyIterator(Action *rootAction, size_t rootIndex)
			: _currentAction(rootAction)
			{
				_indices.push(rootIndex);
			} 

			// _indices contains the path that is followed to reach the _currentAction, starting from the root
			// action. It contains always at least one value. It has one zero value if the iterator is
			// pointing to the root strategy (without children)
			// and contains one value ChildCount of root if the iterator is pointed past the root strategy.
			std::stack<size_t> _indices;

			// _currentAction is the action currently pointed at by the iterator, expect if
			// the iterator is pointed past the end. In that case, it points to the root
			// action.
			Action *_currentAction;
	};

}

#endif
